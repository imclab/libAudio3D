#include <assert.h>
#include <cmath>
#include <stdlib.h>
#include "reberation.h"
#include "fft_filter.h"


Reberation::Reberation(int block_size, int sampling_rate, float reberation_time) :
		block_size_(block_size), reberation_output_read_pos_(0) {
	RenderImpulseResponse(block_size, sampling_rate, reberation_time);

	left_reberation_filter_ = new FFTFilter(block_size_);
	left_reberation_filter_->SetTimeDomainKernel(GetImpulseResponseLeft());
	right_reberation_filter_ = new FFTFilter(block_size_);;
	right_reberation_filter_->SetTimeDomainKernel(GetImpulseResponseRight());

	reberation_input_.reserve(block_size_);
	reberation_output_left_.resize(block_size_, 0.0f);
	reberation_output_right_.resize(block_size_, 0.0f);
}

void Reberation::RenderImpulseResponse(int block_size,
		int sampling_rate, float reberation_time) {
	impulse_response_left_.resize(block_size, 0.0f);
	impulse_response_right_.resize(block_size, 0.0f);

	int quiet_period = block_size;  // filter_delay = block_size
	quiet_period_sec_ = static_cast<float>(quiet_period)/
			static_cast<float>(sampling_rate);
	const float exp_decay = -13.8155;

	srand(0);
	for (int i = 0; i < block_size; ++i) {
		float envelope = exp(
				exp_decay * (i + quiet_period) / sampling_rate
						/ reberation_time);
		assert(envelope >= 0 && envelope<=1.0);
		impulse_response_left_[i] = FloatRand() * envelope;
		impulse_response_right_[i] = FloatRand()*envelope;
	}
}
float Reberation::GetQuietPeriod() const
{
	return quiet_period_sec_;
}

void Reberation::AddReberation(const std::vector<float>& input,
		std::vector<float>* output_left, std::vector<float>* output_right) {
	assert(output_left && output_right);
	assert(output_left->size() == output_right->size());
	assert(input.size() == output_right->size());

	for (int i = 0; i < input.size(); ++i) {
		(*output_left)[i] +=
				reberation_output_left_[reberation_output_read_pos_];
		(*output_right)[i] +=
				reberation_output_right_[reberation_output_read_pos_];
		++reberation_output_read_pos_;
	}
	reberation_input_.insert(reberation_input_.end(), input.begin(),
			input.end());

	if (reberation_output_read_pos_ == block_size_) {
		left_reberation_filter_->AddSignalBlock(reberation_input_);
		right_reberation_filter_->AddSignalBlock(reberation_input_);
		reberation_input_.clear();
		left_reberation_filter_->GetResult(&reberation_output_left_);
		right_reberation_filter_->GetResult(&reberation_output_right_);
		reberation_output_read_pos_ = 0;
	}
	assert(reberation_output_read_pos_<block_size_);
}

float Reberation::FloatRand() {
	return static_cast<float>(rand())/static_cast<float>(RAND_MAX);
}

const std::vector<float>& Reberation::GetImpulseResponseLeft() const{
	return impulse_response_left_;
}

const std::vector<float>& Reberation::GetImpulseResponseRight() const{
return impulse_response_right_;
}


