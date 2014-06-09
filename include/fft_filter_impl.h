#ifndef FFT_FILTER_IMPL_H_
#define FFT_FILTER_IMPL_H_
#include <vector>

#include "common.h"

#include "kiss_fftr.h"

using std::vector;

class FFTFilterImpl {
 public:
  FFTFilterImpl(int filter_len);
  virtual ~FFTFilterImpl();

  void SetTimeDomainKernel(const vector<float>& kernel);

  void SetFreqDomainKernel(const vector<Complex>& kernel);

  void ForwardTransform(const vector<float>& time_signal,
                        vector<Complex>* freq_signal) const;
  void InverseTransform(const vector<Complex>& freq_signal,
                        vector<float>* time_signal) const;

  void AddSignalBlock(const vector<float>& signal_block);

  void GetResult(vector<float>* signal_block);
 private:
  void Init();

  void KissComplexFormatConvert(const vector<kiss_fft_cpx>& input,
                                vector<Complex>* output) const;

  void KissComplexFormatConvert(const vector<Complex>& input,
                                vector<kiss_fft_cpx>* output) const;

  void ComplexVectorProduct(const vector<kiss_fft_cpx>& input_a,
                            const vector<kiss_fft_cpx>& input_b,
                            vector<kiss_fft_cpx>* result) const;

  void VectorCopyWithZeroPadding(const vector<kiss_fft_scalar>& input,
                                 vector<kiss_fft_scalar>* output) const;

  void InverseFFTScaling(vector<float>* signal) const;

  int max_kernel_len_;
  int fft_len_;

  vector<float> filter_state_;
  vector<float> window_;

  vector<kiss_fft_scalar> kernel_time_domain_buffer_;
  vector<kiss_fft_cpx> kernel_freq_domain_buffer_;

  int buffer_selector_;
  vector<vector<kiss_fft_scalar> > signal_time_domain_buffer_;
  vector<vector<kiss_fft_cpx> > signal_freq_domain_buffer_;

  vector<kiss_fft_cpx> filtered_freq_domain_buffer_;

  kiss_fftr_cfg forward_fft_;
  kiss_fftr_cfg inverse_fft_;
};

#endif  // FFT_FILTER_IMPL_H_
