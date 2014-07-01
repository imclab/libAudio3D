#ifndef AUDIO_RENDER_
#define AUDIO_RENDER_

#include <vector>
#include "common.h"
#include "config.h"

class HRTF;
class FFTFilter;
class RoomModel;

class AudioRender {
 public:
  AudioRender(const Audio3DConfigT& config, RoomModel* room_model);
  virtual ~AudioRender();


  void RenderAudio(const std::vector<float>&input,
                     std::vector<float>* output_left,
                     std::vector<float>* output_right);

  private:
  static const std::vector<AcousticSource> RenderReflections(const RoomModel& room_model, int reflection_order);

  const Audio3DConfigT& config_;

  RoomModel* room_model_;  // Not owned

  std::vector<AcousticSource> acoustic_sources_;

  HRTF* hrtf_lookup_;
  std::vector<HRTFInfo> hftf_infos_;

  FFTFilter* left_fft_filter_;
  FFTFilter* right_fft_filter_;


};

#endif  // AUDIO_RENDER_

