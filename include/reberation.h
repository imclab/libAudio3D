#ifndef REBERATION_H_
#define REBERATION_H_

#include <vector>
class FFTFilter;
class Reberation {
 public:
  Reberation(int block_size, int sampling_rate, float reberation_time);
  float GetQuietPeriod() const;

  void RenderReberation(const std::vector<float>& input,
                        std::vector<float>* output_left,
                        std::vector<float>* output_right);

  const std::vector<float>& GetImpulseResponseLeft() const;
  const std::vector<float>& GetImpulseResponseRight() const;

 private:
  void RenderImpulseResponse(int block_size, int sampling_rate,
                             float reberation_time);
  static float FloatRand();
  int block_size_;
  std::vector<float> impulse_response_left_;
  std::vector<float> impulse_response_right_;
  float quiet_period_sec_;

  FFTFilter* left_reberation_filter_;
  FFTFilter* right_reberation_filter_;
  std::vector<float> reberation_input_;
  std::vector<float> reberation_output_left_;
  std::vector<float> reberation_output_right_;
  int reberation_output_read_pos_;

};

#endif

