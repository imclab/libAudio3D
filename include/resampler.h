#ifndef RESAMPLE_HRTF
#define RESAMPLE_HRTF

#include <vector>

struct SRC_STATE_tag;

class Resampler {
 public:
  Resampler(int input_len, double resampling_factor);
  ~Resampler();

  void Resample(const std::vector<float>& intput, std::vector<float>* output);
  int GetOutputLength();

 private:
  int input_len_;
  int output_len_;

  double resample_factor_;
  SRC_STATE_tag* libsamplerate_handle_;
};

#endif
