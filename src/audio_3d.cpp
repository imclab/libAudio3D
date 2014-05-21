#include <cmath>
#include <assert.h>
#include "audio_3d.h"
#include "hrtf.h"
#include "fft_filter.h"
#include "reberation.h"

Audio3DSource::Audio3DSource(int sample_rate, int block_size)
    : sample_rate_(sample_rate),
      block_size_(block_size),
      elevation_deg_(0.0f),
      azimuth_deg_(0.0f),
      distance_(0.0f),
      hrtf_(0),
      left_hrtf_filter_(0),
      right_hrtf_filter_(0) {
  prev_signal_block_.resize(block_size, 0.0f);

  CalculateXFadeWindow();

  hrtf_ = new HRTF(sample_rate, block_size_);
  left_hrtf_filter_ = new FFTFilter(block_size_);
  right_hrtf_filter_ = new FFTFilter(block_size_);

  left_hrtf_filter_->SetFreqDomainKernel(hrtf_->GetLeftEarFreqHRTF());
  right_hrtf_filter_->SetFreqDomainKernel(hrtf_->GetRightEarFreqHRTF());

  int reberation_size = 2048 * 2;
  float reberation_duration = 0.100;
  reberation_ = new Reberation(reberation_size, sample_rate_,
                               reberation_duration);
}

Audio3DSource::~Audio3DSource() {
  delete hrtf_;
  delete left_hrtf_filter_;
  delete right_hrtf_filter_;
  delete reberation_;
}

void Audio3DSource::SetPosition(int x, int y, int z) {
}
void Audio3DSource::SetDirection(float elevation_deg, float azimuth_deg,
                                 float distance) {
  elevation_deg_ = elevation_deg;
  azimuth_deg_ = azimuth_deg;

  float hrft_distance = hrtf_->GetDistance();
  distance_ = fmax(distance, hrft_distance);
  damping_ = hrft_distance / distance_;
  assert(damping_ >= 0 && damping_ <= 1.0f);
}

void Audio3DSource::CalculateXFadeWindow() {
  xfade_window_.resize(block_size_);
  double phase_step = M_PI / 2.0 / (block_size_ - 1);
  for (int i = 0; i < block_size_; ++i) {
    xfade_window_[i] = sin(i * phase_step);
    xfade_window_[i] *= xfade_window_[i];
  }
}

void Audio3DSource::ProcessBlock(const std::vector<float>&input,
                                 std::vector<float>* output_left,
                                 std::vector<float>* output_right) {
  assert(output_left != 0 && output_right != 0);

  left_hrtf_filter_->AddSignalBlock(input);

  std::vector<float> current_hrtf_output_left;
  left_hrtf_filter_->GetResult(&current_hrtf_output_left);

  right_hrtf_filter_->AddSignalBlock(input);

  std::vector<float> current_hrtf_output_right;
  right_hrtf_filter_->GetResult(&current_hrtf_output_right);

  bool new_hrtf_selected = hrtf_->SetDirection(elevation_deg_, azimuth_deg_);
  if (!new_hrtf_selected) {
    output_left->swap(current_hrtf_output_left);
    output_right->swap(current_hrtf_output_right);
  } else {
    // Update filter kernels
    left_hrtf_filter_->SetFreqDomainKernel(hrtf_->GetLeftEarFreqHRTF());
    right_hrtf_filter_->SetFreqDomainKernel(hrtf_->GetRightEarFreqHRTF());
    // Update filter state with previous signal block
    left_hrtf_filter_->AddSignalBlock(prev_signal_block_);
    right_hrtf_filter_->AddSignalBlock(prev_signal_block_);

    // Filter current input with updated HRTF filters.
    left_hrtf_filter_->AddSignalBlock(input);
    right_hrtf_filter_->AddSignalBlock(input);

    std::vector<float> updated_hrtf_output_left;
    left_hrtf_filter_->GetResult(&updated_hrtf_output_left);
    std::vector<float> updated_hrtf_output_right;
    right_hrtf_filter_->GetResult(&updated_hrtf_output_right);

    output_left->resize(block_size_);
    ApplyXFadeWindow(current_hrtf_output_left, updated_hrtf_output_left,
                     output_left);
    output_right->resize(block_size_);
    ApplyXFadeWindow(current_hrtf_output_right, updated_hrtf_output_right,
                     output_right);
  }

  prev_signal_block_ = input;

  ApplyDamping(damping_, output_left);
  ApplyDamping(damping_, output_right);

  // Reberation damping!!
  reberation_->RenderReberation(input, output_left, output_right);
}

void Audio3DSource::ApplyXFadeWindow(const std::vector<float>& block_a,
                                     const std::vector<float>& block_b,
                                     std::vector<float>* output) {
  assert(output != 0);
  assert(block_a.size() == block_b.size());
  assert(block_a.size() == xfade_window_.size());
  assert(output->size() == xfade_window_.size());

  for (int i = 0; i < block_a.size(); ++i) {
    (*output)[i] = block_a[i] * xfade_window_[xfade_window_.size() - 1 - i]
        + block_b[i] * xfade_window_[i];
  }
}

void Audio3DSource::ApplyDamping(float damping_factor,
                                 std::vector<float>* block) {
  assert(block != 0);
  for (int i = 0; i < block->size(); ++i) {
    (*block)[i] *= damping_factor;
  }
}

