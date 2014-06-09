#include <assert.h>
#include <cmath>

#include "fft_filter.h"
#include "fft_filter_impl.h"

using namespace std;

FFTFilter::FFTFilter(int filter_len)
    : fft_filter_impl_(new FFTFilterImpl(filter_len)) {
}

FFTFilter::~FFTFilter() {
  delete fft_filter_impl_;
}

void FFTFilter::SetTimeDomainKernel(const std::vector<float>& kernel) {
  fft_filter_impl_->SetTimeDomainKernel(kernel);
}

void FFTFilter::SetFreqDomainKernel(const std::vector<Complex>& kernel) {
  fft_filter_impl_->SetFreqDomainKernel(kernel);
}

void FFTFilter::ForwardTransform(const vector<float>& time_signal,
                                 vector<Complex>* freq_signal) const {
  fft_filter_impl_->ForwardTransform(time_signal, freq_signal);
}

void FFTFilter::InverseTransform(const vector<Complex>& freq_signal,
                                 vector<float>* time_signal) const {
  fft_filter_impl_->InverseTransform(freq_signal, time_signal);
}

void FFTFilter::AddSignalBlock(const vector<float>& signal_block) {
  fft_filter_impl_->AddSignalBlock(signal_block);
}

void FFTFilter::GetResult(vector<float>* signal_block) {
  fft_filter_impl_->GetResult(signal_block);
}


