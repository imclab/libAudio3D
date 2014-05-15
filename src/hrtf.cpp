#include "hrtf_data.h"
#include "fft_filter.h"
#include "hrtf.h"
#include "resampler.h"
#include "flann_nn_search.hpp"

HRTF::HRTF(int sample_rate, int block_size)
    : sample_rate_(sample_rate),
      block_size_(block_size),
      hrtf_index_(-1),
      hrtf_elevation_deg_(-1.0),
      hrtf_azimuth_deg_(-1.0),
      left_right_swap_(false),
      filter_size_(-1) {
  hrtf_nn_search_ = new FLANNNeighborSearch();

  InitNeighborSearch();
  ResampleHRTFs();
  FreqTransformHRTFs();
  SetDirection(0.0f, 0.0f);
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

bool HRTF::SetDirection(float elevation_deg, float azimuth_deg) {
  if (elevation_deg == hrtf_elevation_deg_
      && azimuth_deg == hrtf_azimuth_deg_) {
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

  if (hrtf_index_ == hrtf_index) {
    return false;
  }
  hrtf_index_ = hrtf_index;

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

void HRTF::GetDirection(float* elevation_deg, float* azimuth_deg) const {
  assert(elevation_deg && azimuth_deg);
  *elevation_deg = hrtf_elevation_deg_;
  *azimuth_deg = hrtf_azimuth_deg_;
}

const std::vector<float>& HRTF::GetLeftEarTimeHRTF() const {
  assert(hrtf_index_ >= 0 && hrtf_index_ < kHRTFDataSet.num_hrtfs);
  return
      left_right_swap_ ?
          hrtf_resampled_time_domain_[hrtf_index_].second :
          hrtf_resampled_time_domain_[hrtf_index_].first;
}
const std::vector<float>& HRTF::GetRightEarTimeHRTF() const {
  assert(hrtf_index_ >= 0 && hrtf_index_ < kHRTFDataSet.num_hrtfs);
  return
      left_right_swap_ ?
          hrtf_resampled_time_domain_[hrtf_index_].first :
          hrtf_resampled_time_domain_[hrtf_index_].second;
}

const std::vector<float>& HRTF::GetLeftEarFreqHRTF() const {
  assert(hrtf_index_ >= 0 && hrtf_index_ < kHRTFDataSet.num_hrtfs);
  return
      left_right_swap_ ?
          hrtf_resampled_freq_domain_[hrtf_index_].second :
          hrtf_resampled_freq_domain_[hrtf_index_].first;
}
const std::vector<float>& HRTF::GetRightEarFreqHRTF() const {
  assert(hrtf_index_ >= 0 && hrtf_index_ < kHRTFDataSet.num_hrtfs);
  return
      left_right_swap_ ?
          hrtf_resampled_freq_domain_[hrtf_index_].first :
          hrtf_resampled_freq_domain_[hrtf_index_].second;
}

float HRTF::GetDistance() const {
  return kHRTFDataSet.distance;
}

int HRTF::GetFilterSize() const {
  return filter_size_;
}

void HRTF::ResampleHRTFs() {
  double resample_factor = static_cast<double>(sample_rate_)
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

void HRTF::FreqTransformHRTFs() {
  FFTFilter fft_filter(block_size_);
  hrtf_resampled_freq_domain_.resize(kHRTFDataSet.num_hrtfs);
  for (int hrtf_itr = 0; hrtf_itr < kHRTFDataSet.num_hrtfs; ++hrtf_itr) {
    fft_filter.ForwardTransform(hrtf_resampled_time_domain_[hrtf_itr].first,
                                &hrtf_resampled_freq_domain_[hrtf_itr].first);
    fft_filter.ForwardTransform(hrtf_resampled_time_domain_[hrtf_itr].second,
                                &hrtf_resampled_freq_domain_[hrtf_itr].second);
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

