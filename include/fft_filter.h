#ifndef FFT_FILTER_H_
#define FFT_FILTER_H_

#include <vector>
#include "common.h""

using std::vector;

class FFTFilterImpl;

class FFTFilter {
 public:
  FFTFilter(int filter_len);
  virtual ~FFTFilter();

  void SetTimeDomainKernel(const vector<float>& kernel);

  void SetFreqDomainKernel(const vector<Complex>& kernel);

  void ForwardTransform(const vector<float>& time_signal,
                           vector<Complex>* freq_signal) const;
  void InverseTransform(const vector<Complex>& freq_signal,
                           vector<float>* time_signal) const;

  void AddSignalBlock(const vector<float>& signal_block);

  void GetResult(vector<float>* signal_block);
 private:
  FFTFilterImpl* fft_filter_impl_;

};

#endif
