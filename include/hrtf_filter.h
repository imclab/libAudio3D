#ifndef HRTF_FILTER_H_
#define HRTF_FILTER_H_

#include <stdint.h>
#include <vector>

#include "config.h"
#include "common.h"

union Point3D;
class FFTFilter;
class HRTF;
class Reberation;

class HRTFFilter {
 public:
  HRTFFilter(const Audio3DConfigT& config);
  virtual ~HRTFFilter();

  void SetFilterFreqDomain(std::vector<Complex>* left_filter,
                           std::vector<Complex>* right_filter);

  void ProcessBlock(const std::vector<float>&input,
                    std::vector<float>* output_left,
                    std::vector<float>* output_right);
 private:
  void CalculateXFadeWindow();
  void ApplyXFadeWindow(const std::vector<float>& block_a,
                        const std::vector<float>& block_b,
                        std::vector<float>* output);

  const Audio3DConfigT config_;

  std::vector<float> xfade_window_;
  std::vector<float> prev_signal_block_;

  std::vector<Complex> next_hrtf_left_filter_;
  std::vector<Complex> next_hrtf_right_filter_;
  bool switch_filters_;

  FFTFilter* left_hrtf_filter_;
  FFTFilter* right_hrtf_filter_;
};

#endif  // HRTF_FILTER_H_
