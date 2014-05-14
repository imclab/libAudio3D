#include <assert.h>
#include <cmath>
#include <string>
#include <iostream>
#include <cstdlib>

#include <samplerate.h>
#include "resampler.h"

using namespace std;

Resampler::Resampler(int input_len, double resampling_factor)
    : input_len_(input_len),
      resample_factor_(resampling_factor) {
  assert(resample_factor_ > 0);

  int error;
  static const int channels = 1;
  libsamplerate_handle_ = src_new(SRC_SINC_BEST_QUALITY, channels, &error);
  if (!libsamplerate_handle_) {
    cerr << "Error during libsamplerate initialization: "
        << string(src_strerror(error));
    exit(1);
  }

  // Next power of 2 of (input_len_ * resample_factor_)
  //int output_len_bits = static_cast<int>(floor(
  //    log(input_len_ * resample_factor_) / log(2)));
  //output_len_ = 1 << (output_len_bits + 1);
  output_len_ = input_len_ * resample_factor_ + 1;

  assert(libsamplerate_handle_ && "Creating libsamplerate handler failed");
}

Resampler::~Resampler() {
  assert(libsamplerate_handle_);
  libsamplerate_handle_ = src_delete(libsamplerate_handle_);
}

void Resampler::Resample(const vector<float>& input, vector<float>* output) {
  assert(output);
  if (resample_factor_ != 1.0) {
    output->clear();
    output->resize(output_len_, 0.0f);

    SRC_DATA libsamplerate_data;
    libsamplerate_data.data_in = const_cast<float*>(&input[0]);
    libsamplerate_data.input_frames = input.size();
    libsamplerate_data.data_out = &((*output)[0]);
    libsamplerate_data.output_frames = output->size();
    libsamplerate_data.src_ratio = resample_factor_;
    libsamplerate_data.end_of_input = 1;

    int error = src_reset(libsamplerate_handle_);
    if (error != 0) {
      cerr << "Error during libsamplerate reset: "
          << string(src_strerror(error));
      exit(1);
    }
    error = src_set_ratio(libsamplerate_handle_, resample_factor_);
    if (error != 0) {
      cerr << "Error setting resampling ratio: " << string(src_strerror(error));
      exit(1);
    }
    error = src_process(libsamplerate_handle_, &libsamplerate_data);
    if (error != 0) {
      cerr << "Error during resampling: " << string(src_strerror(error));
      exit(1);
    }
  } else {
    // Copy signal if resample factor is 1.0
    output->assign(input.begin(), input.end());
  }
}

int Resampler::GetOutputLength() {
  return output_len_;
}

