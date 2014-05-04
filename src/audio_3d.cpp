#include <cmath>
#include <assert.h>
#include "audio_3d.h"
#include "hrtf.h"
#include "fft_filter.h"

Audio3DSource::Audio3DSource(int sample_rate, int block_size) :
		sample_rate_(sample_rate),
		block_size_(block_size),
		elevation_deg_(0.0f),
		azimuth_deg_(0.0f),
		distance_(0.0f),
		hrtf_(0),
		left_fft_filter_(0),
		right_fft_filter_(0)
{
	prev_signal_block_.resize(block_size, 0.0f);

	CalculateXFadeWindow();

	hrtf_ = new HRTF(sample_rate);
	left_fft_filter_ = new FFTFilter(block_size_);
	right_fft_filter_ = new FFTFilter(block_size_);

	left_fft_filter_->SetKernel(hrtf_->GetLeftEarHRTF());
	right_fft_filter_->SetKernel(hrtf_->GetRightEarHRTF());
}

void Audio3DSource::SetPosition(int x, int y, int z)
{
   if (hrtf_!=0) {
	   delete(hrtf_);
   }
   if (left_fft_filter_!=0) {
	   delete(left_fft_filter_);
   }
   if (right_fft_filter_!=0) {
	   delete(right_fft_filter_);
   }
}
void Audio3DSource::SetDirection(float elevation_deg, float azimuth_deg, float distance) {
	elevation_deg_ = elevation_deg;
	azimuth_deg_ = azimuth_deg;
	distance_ = distance;
}

void Audio3DSource::CalculateXFadeWindow() {
	double phase_step = M_PI/2.0/(block_size_-1);
	for (int i = 0; i < block_size_; ++i) {
		xfade_window_[i] = sin(i*phase_step);
		xfade_window_[i] *= xfade_window_[i];
	}
}

void Audio3DSource::ProcessBlock(const std::vector<float>&input,
		std::vector<float>* output_left, std::vector<float>* output_right) {
	assert(output_left != 0 && output_right != 0);

	left_fft_filter_->AddSignalBlock(input);

	std::vector<float> current_hrtf_output_left;
	left_fft_filter_->GetResult(&current_hrtf_output_left);

	right_fft_filter_->AddSignalBlock(input);

	std::vector<float> current_hrtf_output_right;
	right_fft_filter_->GetResult(&current_hrtf_output_right);

	bool new_hrtf_selected = hrtf_->SetDirection(elevation_deg_, azimuth_deg_);
	if (!new_hrtf_selected) {
		output_left->swap(current_hrtf_output_left);
		output_right->swap(current_hrtf_output_right);
	} else {
		// Update filter kernels
		left_fft_filter_->SetKernel(hrtf_->GetLeftEarHRTF());
		right_fft_filter_->SetKernel(hrtf_->GetRightEarHRTF());
		// Update filter state with previous and current signal block
		left_fft_filter_->AddSignalBlock(prev_signal_block_);
		left_fft_filter_->AddSignalBlock(input);
		right_fft_filter_->AddSignalBlock(prev_signal_block_);
		right_fft_filter_->AddSignalBlock(input);

		std::vector<float> updated_hrtf_output_left;
		left_fft_filter_->GetResult(&updated_hrtf_output_left);
		std::vector<float> updated_hrtf_output_right;
		right_fft_filter_->GetResult(&updated_hrtf_output_right);

		//
		ApplyXFadeWindow(current_hrtf_output_left, updated_hrtf_output_left, output_left);
		ApplyXFadeWindow(current_hrtf_output_right, updated_hrtf_output_right, output_right);
	}

	prev_signal_block_ = input;
}

void Audio3DSource::ApplyXFadeWindow(const std::vector<float>& block_a,
		const std::vector<float>& block_b, std::vector<float>* output) {
	assert(output != 0);
	assert(block_a.size() == block_b.size());
	assert(block_a.size() == xfade_window_.size());

	for (int i = 0; i < block_a.size(); ++i) {
		(*output)[i] = block_a[i] * xfade_window_[xfade_window_.size() - 1 - i]
				+ block_b[i] * xfade_window_[i];
	}
}

void Audio3DSource::ApplyDamping(float damping_factor, std::vector<float>* block) {
	assert(block!=0);
	for (int i=0; i<block->size(); ++i) {
		(*block)[i] *= damping_factor;
	}
}


