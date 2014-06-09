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
  AudioRender(const Audio3DConfigT& config, const RoomModel* room_model);
  virtual ~AudioRender();

  void SetSourcePosition(const Point3D& source_pos);
  void SetListenerPosition(const Point3D& listener_pos);

  void RenderAudio(const std::vector<float>&input,
                     std::vector<float>* output_left,
                     std::vector<float>* output_right);

  private:
  void Init();
  const std::vector<AcousticSource> RenderReflections(const std::vector<WallModel>& walls) const;

  const Audio3DConfigT& config_;

  const RoomModel* room_model_;  // Not owned

  Point3D source_pos_;
  Point3D listener_pos_;
  bool source_listener_pos_changed_;

  std::vector<AcousticSource> acoustic_sources_;

  HRTF* hrtf_lookup_;
  std::vector<HRTFInfo> hftf_infos_;

  FFTFilter* left_fft_filter_;
  FFTFilter* right_fft_filter_;


};

#endif  // AUDIO_RENDER_

