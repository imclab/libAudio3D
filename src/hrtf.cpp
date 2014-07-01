#include <cmath>

#include "hrtf_data.h"
#include "fft_filter.h"
#include "hrtf.h"
#include "resampler.h"
#include "flann_nn_search.hpp"

HRTF::HRTF(const Audio3DConfigT& config)
    : config_(config),
      hrtf_index_(-1),
      distance_index_(0),
      hrtf_elevation_deg_(-1.0),
      hrtf_azimuth_deg_(-1.0),
      left_right_swap_(false),
      filter_time_domain_size_(-1) {
  hrtf_nn_search_ = new FLANNNeighborSearch();

  num_distance_intervals_ = (config_.max_distance_meter - kHRTFDataSet.distance)
      / config_.distance_resolution_meter + 1;
  assert(num_distance_intervals_>0);

  InitNeighborSearch();
  ResampleHRTFs();
  PrecalculateFreqHRTFs();
}

HRTF::~HRTF() {
  delete hrtf_nn_search_;
}

void HRTF::InitNeighborSearch() {
  // Add orientations of right hemisphere
  for (int i = 0; i < kHRTFDataSet.num_hrtfs; ++i) {
    float elevation_deg = kHRTFDataSet.direction[i][0];
    float azimuth_deg = kHRTFDataSet.direction[i][1];
    hrtf_nn_search_->AddHRTFDirection(elevation_deg, azimuth_deg, i);
  }
  hrtf_nn_search_->BuildIndex();
}

HRTFInfo HRTF::GetHRTFInfo(const Vec3d_f& source_pos) {
  float distance = Vec3dNorm(source_pos);
  float elevation_deg = acos(source_pos.z / distance);
  float azimuth_deg = atan2(source_pos.y, source_pos.x);
  return GetHRTFInfo(elevation_deg, azimuth_deg, distance);
}

HRTFInfo HRTF::GetHRTFInfo(float elevation_deg, float azimuth_deg,
                             float distance) {
  float min_max_distance = std::min<float>(
      config_.max_distance_meter,
      std::max<float>(distance, kHRTFDataSet.distance));
  int new_distance_index = min_max_distance / config_.distance_resolution_meter;
  assert(new_distance_index<num_distance_intervals_);

  int new_elevation_deg = elevation_deg;
  int new_azimuth_deg = azimuth_deg;
  while (new_azimuth_deg < -180) {
    new_azimuth_deg += 360;
  }
  while (new_azimuth_deg > 180) {
    new_azimuth_deg -= 360;
  }

  int hrtf_index = hrtf_nn_search_->FindNearestHRTF(new_elevation_deg,
                                                    fabs(new_azimuth_deg));
  assert(hrtf_index >= 0 && hrtf_index < kHRTFDataSet.num_hrtfs);

  bool left_right_swap = (new_azimuth_deg < 0.0f);

  HRTFInfo return_value;
  return_value.distance_index=new_distance_index;
  return_value.hrtf_index=hrtf_index;
  return_value.left_right_swap=left_right_swap;
  return return_value;
}

const std::vector<Complex>& HRTF::GetLeftEarFreqHRTF(
    const HRTFInfo& hrtf_info) const {
  assert(
      hrtf_info.hrtf_index >= 0 && hrtf_info.hrtf_index < kHRTFDataSet.num_hrtfs &&
      hrtf_info.distance_index>=0 && hrtf_info.distance_index<num_distance_intervals_);
  return
      hrtf_info.left_right_swap ?
          delayed_hrtf_freq_domain_[hrtf_info.hrtf_index][hrtf_info.distance_index].second :
          delayed_hrtf_freq_domain_[hrtf_info.hrtf_index][hrtf_info.distance_index].first;
}
const std::vector<Complex>& HRTF::GetRightEarFreqHRTF(const HRTFInfo& hrtf_info) const {
  assert(
      hrtf_info.hrtf_index >= 0 && hrtf_info.hrtf_index < kHRTFDataSet.num_hrtfs &&
      hrtf_info.distance_index>=0 && hrtf_info.distance_index<num_distance_intervals_);
  return
      hrtf_info.left_right_swap ?
          delayed_hrtf_freq_domain_[hrtf_info.hrtf_index][hrtf_info.distance_index].first :
          delayed_hrtf_freq_domain_[hrtf_info.hrtf_index][hrtf_info.distance_index].second;
}

int HRTF::GetFreqHRTFSize() const {
  return filter_freq_domain_size_;
}

int HRTF::GetTimeHRTFSize() const {
  return filter_time_domain_size_;
}

void HRTF::ResampleHRTFs() {
  double resample_factor = static_cast<double>(config_.sample_rate)
      / static_cast<double>(kHRTFDataSet.sample_rate);
  Resampler resampler(kHRTFDataSet.fir_length, resample_factor);

  filter_time_domain_size_ = resampler.GetOutputLength();

  std::vector<float> left_hrtf_float(kHRTFDataSet.fir_length);
  std::vector<float> right_hrtf_float(kHRTFDataSet.fir_length);

  hrtf_resampled_time_domain_.resize(kHRTFDataSet.num_hrtfs);
  for (int hrtf_itr = 0; hrtf_itr < kHRTFDataSet.num_hrtfs; ++hrtf_itr) {
// Convert raw HRTF to float vector.
    ConvertShortToFloatVector(&kHRTFDataSet.data[hrtf_itr][0][0],
                              kHRTFDataSet.fir_length, &left_hrtf_float);
    ConvertShortToFloatVector(&kHRTFDataSet.data[hrtf_itr][1][0],
                              kHRTFDataSet.fir_length, &right_hrtf_float);

    // Resample HRTF float vectors to match target sample rate.
    std::vector<float> left_hrtf_resampled_float;
    resampler.Resample(left_hrtf_float, &left_hrtf_resampled_float);
    std::vector<float> right_hrtf_resampled_float;
    resampler.Resample(right_hrtf_float, &right_hrtf_resampled_float);

    // Assign resampled HRTFs to hrtf_resampled_;
    hrtf_resampled_time_domain_[hrtf_itr].first.swap(left_hrtf_resampled_float);
    hrtf_resampled_time_domain_[hrtf_itr].second.swap(
        right_hrtf_resampled_float);
  }
}

void HRTF::PrecalculateFreqHRTFs() {
  filter_freq_domain_size_ = -1;

  FFTFilter fft_filter(config_.block_size);
  delayed_hrtf_freq_domain_.resize(kHRTFDataSet.num_hrtfs);

  for (int hrtf_itr = 0; hrtf_itr < kHRTFDataSet.num_hrtfs; ++hrtf_itr) {
    const ResampledHRTFT& left_hrtf_orig = hrtf_resampled_time_domain_[hrtf_itr]
        .first;
    const ResampledHRTFT& right_hrtf_orig =
        hrtf_resampled_time_domain_[hrtf_itr].second;

    std::vector<ResampledTimeShiftedComplexHRTFTPairT> hrtf_distance_set_freq_domain(
        num_distance_intervals_);
    for (int dist_int = 0; dist_int < num_distance_intervals_; ++dist_int) {
      float distance_meter = kHRTFDataSet.distance
          + dist_int * config_.distance_resolution_meter;
      float sound_travel_time_second = distance_meter
          / kSoundSpeedMeterPerSecond;
      int sound_travel_time_samples = sound_travel_time_second
          * config_.sample_rate;

      float damping = kHRTFDataSet.distance / distance_meter;
      assert(damping >= 0 && damping <= 1.0f);

      ResampledHRTFT left_hrtf_time_shift(left_hrtf_orig.size(), 0.0f);
      for (int i = 0; i + sound_travel_time_samples < left_hrtf_orig.size(); ++i) {
        left_hrtf_time_shift[i + sound_travel_time_samples] = left_hrtf_orig[i]
            * damping;
      }

      ResampledHRTFT right_hrtf_time_shift(right_hrtf_orig.size(), 0.0f);
      for (int i = 0; i + sound_travel_time_samples < right_hrtf_orig.size(); ++i) {
        right_hrtf_time_shift[i + sound_travel_time_samples] =
            right_hrtf_orig[i] * damping;
      }

      fft_filter.ForwardTransform(
          left_hrtf_time_shift, &hrtf_distance_set_freq_domain[dist_int].first);
      fft_filter.ForwardTransform(
          right_hrtf_time_shift,
          &hrtf_distance_set_freq_domain[dist_int].second);

      if (filter_freq_domain_size_ < 0) {
        filter_freq_domain_size_ = hrtf_distance_set_freq_domain[dist_int].first
            .size();
      }
      assert(
          hrtf_distance_set_freq_domain[dist_int].first.size()==filter_freq_domain_size_);
      assert(
          hrtf_distance_set_freq_domain[dist_int].second.size()==filter_freq_domain_size_);
    }

    delayed_hrtf_freq_domain_[hrtf_itr].swap(hrtf_distance_set_freq_domain);

  }
}

void HRTF::ConvertShortToFloatVector(const short* input_ptr, int input_size,
                                     std::vector<float>* output) {
  assert(output != 0);
  assert(input_ptr != 0);
  output->resize(input_size);
  const short* input_raw_ptr = input_ptr;
  for (int i = 0; i < input_size; ++i, ++input_raw_ptr) {
    (*output)[i] = (*input_raw_ptr) / static_cast<float>(0x7FFF);
  }
}

