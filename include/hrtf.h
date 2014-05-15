#ifndef HRTF_LOOKUP_H_
#define HRTF_LOOKUP_H_

#include <memory>
#include <utility>
#include <vector>

class FLANNNeighborSearch;

class HRTF {
 public:
  HRTF(int sample_rate, int block_size);
  virtual ~HRTF();

  bool SetDirection(float elevation_deg, float azimuth_deg);
  void GetDirection(float* elevation_deg, float* azimuth_deg) const;

  const std::vector<float>& GetLeftEarTimeHRTF() const;
  const std::vector<float>& GetRightEarTimeHRTF() const;

  const std::vector<float>& GetLeftEarFreqHRTF() const;
  const std::vector<float>& GetRightEarFreqHRTF() const;

  float GetDistance() const;

  int GetFilterSize() const;

 private:
  static void ConvertShortToFloatVector(const short* input_ptr, int input_size,
                                        std::vector<float>* output);

  void ResampleHRTFs();
  void FreqTransformHRTFs();
  void InitNeighborSearch();

  int sample_rate_;
  int block_size_;

  FLANNNeighborSearch* hrtf_nn_search_;

  int hrtf_index_;
  bool left_right_swap_;

  float hrtf_elevation_deg_;
  float hrtf_azimuth_deg_;

  int filter_size_;

  typedef std::vector<float> ResampledHRTFT;
  typedef std::pair<ResampledHRTFT, ResampledHRTFT> ResampledHRTFPairT;
  std::vector<ResampledHRTFPairT> hrtf_resampled_time_domain_;
  std::vector<ResampledHRTFPairT> hrtf_resampled_freq_domain_;

};

#endif  // HRTF_LOOKUP_H_
