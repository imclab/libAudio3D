#ifndef HRTF_LOOKUP_H_
#define HRTF_LOOKUP_H_

#include <memory>
#include <utility>
#include <vector>

#include <config.h>

class FLANNNeighborSearch;

class HRTF {
 public:
  HRTF(const Audio3DConfigT& config);
  virtual ~HRTF();

  bool SetSourceDirection(float elevation_deg, float azimuth_deg,
                             float distance);

  const std::vector<float>& GetLeftEarFreqHRTF() const;
  const std::vector<float>& GetRightEarFreqHRTF() const;

  float GetDistance() const;

  int GetFilterSize() const;

 private:
  static void ConvertShortToFloatVector(const short* input_ptr, int input_size,
                                        std::vector<float>* output);

  void ResampleHRTFs();
  void PrecalculateFreqHRTFs();
  void InitNeighborSearch();

  const Audio3DConfigT config_;

  int num_distance_intervals_;

  FLANNNeighborSearch* hrtf_nn_search_;

  int hrtf_index_;
  int distance_index_;
  bool left_right_swap_;

  float hrtf_elevation_deg_;
  float hrtf_azimuth_deg_;

  int filter_size_;

  typedef std::vector<float> ResampledHRTFT;
  typedef std::pair<ResampledHRTFT, ResampledHRTFT> ResampledHRTFPairT;
  std::vector<ResampledHRTFPairT> hrtf_resampled_time_domain_;
  std::vector<std::vector<ResampledHRTFPairT> > hrtf_with_distance_freq_domain_;

};

#endif  // HRTF_LOOKUP_H_
