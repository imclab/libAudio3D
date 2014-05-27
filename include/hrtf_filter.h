#ifndef HRTF_FILTER_H_
#define HRTF_FILTER_H_

#include <stdint.h>
#include <vector>

#include "config.h"

class FFTFilter;
class HRTF;
class Reberation;

class HRTFFilter {
 public:
  HRTFFilter(const Audio3DConfigT& config);
  virtual ~HRTFFilter();

  void SetSourcePosition(int x, int y, int z);
  void SetSourceDirection(float elevation_deg, float azimuth_deg,
                             float distance);

  void ProcessBlock(const std::vector<float>&input,
                    std::vector<float>* output_left,
                    std::vector<float>* output_right);
 private:
  void CalculateXFadeWindow();
  void ApplyXFadeWindow(const std::vector<float>& block_a,
                        const std::vector<float>& block_b,
                        std::vector<float>* output);

  const Audio3DConfigT config_;

  float elevation_deg_;
  float azimuth_deg_;
  float distance_;

  float damping_;

  std::vector<float> xfade_window_;
  std::vector<float> prev_signal_block_;

  HRTF* hrtf_;
  FFTFilter* left_hrtf_filter_;
  FFTFilter* right_hrtf_filter_;

  Reberation* reberation_;
};

#endif  // HRTF_FILTER_H_
