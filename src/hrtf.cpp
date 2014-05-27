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
      filter_size_(-1) {
  hrtf_nn_search_ = new FLANNNeighborSearch();

  num_distance_intervals_ = (config_.max_distance_meter - kHRTFDataSet.distance)
      / config_.distance_resolution_meter + 1;
  assert(num_distance_intervals_>0);

  InitNeighborSearch();
  ResampleHRTFs();
  PrecalculateFreqHRTFs();
  SetSourceDirection(0.0f, 0.0f, 0.0f);
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

bool HRTF::SetSourceDirection(float elevation_deg, float azimuth_deg,
                             float distance) {
  int new_distance_index = std::min<float>(
      config_.max_distance_meter,
      std::max<float>(distance, kHRTFDataSet.distance))
      / config_.distance_resolution_meter;
  assert(new_distance_index<num_distance_intervals_);

  if (elevation_deg == hrtf_elevation_deg_ && azimuth_deg == hrtf_azimuth_deg_
      && new_distance_index == distance_index_) {
    return false;
  }

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

  if (hrtf_index_ == hrtf_index && new_distance_index == distance_index_) {
    return false;
  }
  hrtf_index_ = hrtf_index;
  distance_index_ = new_distance_index;

  // Right hemisphere
  hrtf_elevation_deg_ = kHRTFDataSet.direction[hrtf_index][0];
  hrtf_azimuth_deg_ = kHRTFDataSet.direction[hrtf_index][1];
  left_right_swap_ = (new_azimuth_deg < 0.0f);

  if (left_right_swap_) {
    // Left hemisphere corrections
    hrtf_azimuth_deg_ = kHRTFDataSet.direction[hrtf_index][1] * -1;
  }

  return true;
}

const std::vector<float>& HRTF::GetLeftEarFreqHRTF() const {
  assert(
      hrtf_index_ >= 0 && hrtf_index_ < kHRTFDataSet.num_hrtfs && distance_index_>=0 && distance_index_<num_distance_intervals_);
  return
      left_right_swap_ ?
          hrtf_with_distance_freq_domain_[hrtf_index_][distance_index_].second :
          hrtf_with_distance_freq_domain_[hrtf_index_][distance_index_].first;
}
const std::vector<float>& HRTF::GetRightEarFreqHRTF() const {
  assert(
      hrtf_index_ >= 0 && hrtf_index_ < kHRTFDataSet.num_hrtfs && distance_index_>=0 && distance_index_<num_distance_intervals_);
  return
      left_right_swap_ ?
          hrtf_with_distance_freq_domain_[hrtf_index_][distance_index_].first :
          hrtf_with_distance_freq_domain_[hrtf_index_][distance_index_].second;
}

float HRTF::GetDistance() const {
  return kHRTFDataSet.distance;
}

int HRTF::GetFilterSize() const {
  return filter_size_;
}

void HRTF::ResampleHRTFs() {
  double resample_factor = static_cast<double>(config_.sample_rate)
      / static_cast<double>(kHRTFDataSet.sample_rate);
  Resampler resampler(kHRTFDataSet.fir_length, resample_factor);

  filter_size_ = resampler.GetOutputLength();

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
  FFTFilter fft_filter(config_.block_size);
  hrtf_with_distance_freq_domain_.resize(kHRTFDataSet.num_hrtfs);

  for (int hrtf_itr = 0; hrtf_itr < kHRTFDataSet.num_hrtfs; ++hrtf_itr) {
    const ResampledHRTFT& left_hrtf_orig = hrtf_resampled_time_domain_[hrtf_itr]
        .first;
    const ResampledHRTFT& right_hrtf_orig =
        hrtf_resampled_time_domain_[hrtf_itr].second;

    std::vector<ResampledHRTFPairT> hrtf_distance_freq_domain_(
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

      fft_filter.ForwardTransform(left_hrtf_time_shift,
                                  &hrtf_distance_freq_domain_[dist_int].first);
      fft_filter.ForwardTransform(right_hrtf_time_shift,
                                  &hrtf_distance_freq_domain_[dist_int].second);
    }

    hrtf_with_distance_freq_domain_[hrtf_itr].swap(
        hrtf_distance_freq_domain_);

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

