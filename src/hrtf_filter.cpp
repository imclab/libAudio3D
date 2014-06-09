#include <cmath>
#include <assert.h>
#include "hrtf_filter.h"
#include "hrtf.h"
#include "fft_filter.h"
#include "reberation.h"
#include "common.h"

HRTFFilter::HRTFFilter(const Audio3DConfigT& config)
    : config_(config),
      switch_filters_(false),
      left_hrtf_filter_(0),
      right_hrtf_filter_(0) {
  prev_signal_block_.resize(config_.block_size, 0.0f);

  CalculateXFadeWindow();

  left_hrtf_filter_ = new FFTFilter(config_.block_size);
  right_hrtf_filter_ = new FFTFilter(config_.block_size);
}

HRTFFilter::~HRTFFilter() {
  delete left_hrtf_filter_;
  delete right_hrtf_filter_;
}

void HRTFFilter::SetFilterFreqDomain(std::vector<Complex>* left_filter,
                                     std::vector<Complex>* right_filter) {
  assert(left_filter&&right_filter);
  next_hrtf_left_filter_.swap(*left_filter);
  next_hrtf_right_filter_.swap(*right_filter);
  switch_filters_ = true;
}

void HRTFFilter::CalculateXFadeWindow() {
  xfade_window_.resize(config_.block_size);
  double phase_step = M_PI / 2.0 / config_.block_size;
  for (int i = 0; i < config_.block_size; ++i) {
    xfade_window_[i] = sin(i * phase_step);
    xfade_window_[i] *= xfade_window_[i];
  }
}

void HRTFFilter::ProcessBlock(const std::vector<float>&input,
                                 std::vector<float>* output_left,
                                 std::vector<float>* output_right) {
  assert(output_left != 0 && output_right != 0);

  left_hrtf_filter_->AddSignalBlock(input);
  right_hrtf_filter_->AddSignalBlock(input);

  std::vector<float> current_hrtf_output_left;
  left_hrtf_filter_->GetResult(&current_hrtf_output_left);
  std::vector<float> current_hrtf_output_right;
  right_hrtf_filter_->GetResult(&current_hrtf_output_right);

  if (switch_filters_) {
    switch_filters_ = false;
    // Update filter kernels
    left_hrtf_filter_->SetFreqDomainKernel(next_hrtf_left_filter_);
    right_hrtf_filter_->SetFreqDomainKernel(next_hrtf_right_filter_);
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

    output_left->resize(config_.block_size);
    ApplyXFadeWindow(current_hrtf_output_left, updated_hrtf_output_left,
                     output_left);
    output_right->resize(config_.block_size);
    ApplyXFadeWindow(current_hrtf_output_right, updated_hrtf_output_right,
                     output_right);
  } else {
    output_left->swap(current_hrtf_output_left);
    output_right->swap(current_hrtf_output_right);

  }
  prev_signal_block_ = input;
}

void HRTFFilter::ApplyXFadeWindow(const std::vector<float>& block_a,
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


