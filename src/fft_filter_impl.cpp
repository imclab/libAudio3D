#include <assert.h>
#include <cmath>
#include <string>
#include "fft_filter_impl.h"

using namespace std;

FFTFilterImpl::FFTFilterImpl(int filter_len)
    : max_kernel_len_(filter_len),
      fft_len_(filter_len * 2),
      filter_state_(filter_len, 0.0f),
      kernel_time_domain_buffer_(fft_len_),
      kernel_freq_domain_buffer_(fft_len_ / 2 + 1),
      buffer_selector_(0),
      signal_time_domain_buffer_(2, vector<kiss_fft_scalar>(fft_len_)),
      signal_freq_domain_buffer_(2, vector < kiss_fft_cpx > (fft_len_ / 2 + 1)),
      filtered_freq_domain_buffer_(fft_len_ / 2 + 1) {
  bool is_power_of_two = ((fft_len_ != 0) && !(fft_len_ & (fft_len_ - 1)));
  assert(is_power_of_two && "Filter length must be a power of 2");

  forward_fft_ = kiss_fftr_alloc(fft_len_, 0, 0, 0);
  inverse_fft_ = kiss_fftr_alloc(fft_len_, 1, 0, 0);

  Init();
}

FFTFilterImpl::~FFTFilterImpl() {
  kiss_fft_free(forward_fft_);
  kiss_fft_free(inverse_fft_);
}

void FFTFilterImpl::Init() {
  // Initialize all buffers with zeros.
  memset(&kernel_time_domain_buffer_[0], 0, sizeof(kiss_fft_scalar) * kernel_time_domain_buffer_.size());
  memset(&kernel_freq_domain_buffer_[0], 0,
         sizeof(kiss_fft_cpx) * kernel_freq_domain_buffer_.size());
  for (int i = 0; i < 2; ++i) {
    memset(&signal_time_domain_buffer_[i][0], 0,
           sizeof(kiss_fft_scalar) * signal_time_domain_buffer_.size());
    memset(&signal_freq_domain_buffer_[i][0], 0,
           sizeof(kiss_fft_cpx) * signal_freq_domain_buffer_.size());
  }
}

void FFTFilterImpl::ForwardTransform(const vector<float>& time_signal,
                                     vector<Complex>* freq_signal) const {
  assert(freq_signal);
  assert(
      time_signal.size() <= max_kernel_len_ && "Kernel size must be <= max_kernel_len_");

  vector<kiss_fft_scalar> time_domain_buffer(fft_len_);
  VectorCopyWithZeroPadding(time_signal, &time_domain_buffer);

  vector<kiss_fft_cpx> freq_domain_buffer(fft_len_ / 2 + 1);

  // Perform forward FFT transform
  kiss_fftr(forward_fft_, &time_domain_buffer[0], &freq_domain_buffer[0]);

  freq_signal->resize(freq_domain_buffer.size());
  for (int freq_c = 0; freq_c < freq_domain_buffer.size(); ++freq_c) {
    (*freq_signal)[freq_c].real(freq_domain_buffer[freq_c].r);
    (*freq_signal)[freq_c].imag(freq_domain_buffer[freq_c].i);
  }
}
void FFTFilterImpl::InverseTransform(const vector<Complex>& freq_signal,
                                     vector<float>* time_signal) const {
  assert(time_signal);
  assert(
      freq_signal.size() == fft_len_ / 2 + 1 && "Frequency domain signal must match fft_len_+2");

  vector<kiss_fft_cpx> freq_domain_buffer;
  KissComplexFormatConvert(freq_signal, &freq_domain_buffer);

  time_signal->resize(fft_len_);
  // Perform inverse FFT transform of filtered_freq_domain_buffer_ and store result back in signal_time_domain_buffer_
  kiss_fftri(inverse_fft_, &freq_domain_buffer[0], &(*time_signal)[0]);

  // Invert FFT scaling
  InverseFFTScaling(time_signal);
}

void FFTFilterImpl::InverseFFTScaling(vector<float>* signal) const {
  assert(signal);
  assert(signal->size() == fft_len_);
  // Invert FFT scaling
  for (int i = 0; i < fft_len_; ++i) {
    (*signal)[i] /= fft_len_;
  }
}

void FFTFilterImpl::SetTimeDomainKernel(const std::vector<float>& kernel) {
  assert(
      kernel.size() <= max_kernel_len_ && "Kernel size must be <= max_kernel_len_");

  VectorCopyWithZeroPadding(kernel, &kernel_time_domain_buffer_);

  // Perform forward FFT transform
  kiss_fftr(forward_fft_, &kernel_time_domain_buffer_[0],
            &kernel_freq_domain_buffer_[0]);
}

void FFTFilterImpl::SetFreqDomainKernel(const std::vector<Complex>& kernel) {
  assert(kernel.size() == fft_len_ + 2);

  KissComplexFormatConvert(kernel, &kernel_freq_domain_buffer_);;
}

void FFTFilterImpl::VectorCopyWithZeroPadding(
    const vector<kiss_fft_scalar>& input,
    vector<kiss_fft_scalar>* output) const {
  assert(output);
  assert(input.size() <= output->size());
  memcpy(&((*output)[0]), &input[0], sizeof(kiss_fft_scalar) * input.size());
  memset(&((*output)[input.size()]), 0,
         sizeof(kiss_fft_scalar) * (output->size() - input.size()));
}

void FFTFilterImpl::AddSignalBlock(const vector<float>& signal_block) {
  assert(
      signal_block.size() == max_kernel_len_ && "Signal block size must match filter length");

  // Switch buffer selector
  buffer_selector_ = !buffer_selector_;

  vector<kiss_fft_scalar>& time_domain_buffer =
      signal_time_domain_buffer_[buffer_selector_];
  vector<kiss_fft_cpx>& freq_domain_buffer =
      signal_freq_domain_buffer_[buffer_selector_];

  VectorCopyWithZeroPadding(signal_block, &time_domain_buffer);

  // Perform forward FFT transform
  kiss_fftr(forward_fft_, &time_domain_buffer[0], &freq_domain_buffer[0]);

  // Complex vector product in frequency domain with transformed kernel.
  ComplexVectorProduct(freq_domain_buffer, kernel_freq_domain_buffer_,
                       &filtered_freq_domain_buffer_);

  // Perform inverse FFT transform of filtered_freq_domain_buffer_ and store result back in signal_time_domain_buffer_
  kiss_fftri(inverse_fft_, &filtered_freq_domain_buffer_[0],
             &time_domain_buffer[0]);

  // Invert FFT scaling
  InverseFFTScaling(&time_domain_buffer);
}

void FFTFilterImpl::GetResult(vector<float>* signal_block) {
  assert(signal_block);
  signal_block->resize(max_kernel_len_);

  int curr_buf = buffer_selector_;
  int prev_buf = !buffer_selector_;
  for (int i = 0; i < max_kernel_len_; ++i) {
    (*signal_block)[i] = signal_time_domain_buffer_[curr_buf][i]
        + signal_time_domain_buffer_[prev_buf][i + max_kernel_len_];  // Add overlap from previous FFT transform.
  }
}

void FFTFilterImpl::KissComplexFormatConvert(const vector<kiss_fft_cpx>& input,
                                             vector<Complex>* output) const {
  assert(output);
  output->clear();
  output->reserve(input.size());
  for (int i = 0; i < input.size(); ++i) {
    const kiss_fft_cpx& kiss_complex = input[i];
    output->push_back(Complex(kiss_complex.r, kiss_complex.i));
  }
}

void FFTFilterImpl::KissComplexFormatConvert(
    const vector<Complex>& input, vector<kiss_fft_cpx>* output) const {
  assert(output);
  output->clear();
  output->resize(input.size());
  for (int i = 0; i < input.size(); ++i) {
    const Complex& std_complex = input[i];
    kiss_fft_cpx& kiss_complex = (*output)[i];
    kiss_complex.r = std_complex.real();
    kiss_complex.i = std_complex.imag();
  }
}

void FFTFilterImpl::ComplexVectorProduct(const vector<kiss_fft_cpx>& input_a,
                                         const vector<kiss_fft_cpx>& input_b,
                                         vector<kiss_fft_cpx>* result) const {
  assert(result);
  assert(input_a.size() == input_b.size());

  result->resize(input_a.size());
  for (int i = 0; i < result->size(); ++i) {
    float result_real = input_a[i].r * input_b[i].r
        - input_a[i].i * input_b[i].i;
    float result_imag = input_a[i].r * input_b[i].i
        + input_a[i].i * input_b[i].r;
    (*result)[i].r = result_real;
    (*result)[i].i = result_imag;
  }
}

