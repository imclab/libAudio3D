#ifndef HRTF_LOOKUP_H_
#define HRTF_LOOKUP_H_

#include <memory>
#include <utility>
#include <vector>

#include "config.h"
#include "common.h"

class FLANNNeighborSearch;

class HRTF {
 public:
  HRTF(const Audio3DConfigT& config);
  virtual ~HRTF();

  HRTFInfo GetHRTFInfo(const Vec3d_f& source_pos);
  HRTFInfo GetHRTFInfo(float elevation_deg, float azimuth_deg, float distance);

  const std::vector<Complex>& GetLeftEarFreqHRTF(const HRTFInfo& hrtf_info) const;
  const std::vector<Complex>& GetRightEarFreqHRTF(const HRTFInfo& hrtf_info) const;

  int GetTimeHRTFSize() const;
  int GetFreqHRTFSize() const;

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

  int filter_time_domain_size_;
  int filter_freq_domain_size_;

  typedef std::vector<float> ResampledHRTFT;
  typedef std::pair<ResampledHRTFT, ResampledHRTFT> ResampledHRTFPairT;
  std::vector<ResampledHRTFPairT> hrtf_resampled_time_domain_;

  typedef std::vector<Complex> ResampledTimeShiftedComplexHRTFT;
  typedef std::pair<ResampledTimeShiftedComplexHRTFT,
      ResampledTimeShiftedComplexHRTFT> ResampledTimeShiftedComplexHRTFTPairT;
  std::vector<std::vector<ResampledTimeShiftedComplexHRTFTPairT> > delayed_hrtf_freq_domain_;

};

#endif  // HRTF_LOOKUP_H_
