#include <assert.h>

#include "audio_render.h"
#include "room_model.h"
#include "hrtf.h"
#include "fft_filter.h"

AudioRender::AudioRender(const Audio3DConfigT& config, const RoomModel* room_model)
    : config_(config),
      room_model_(room_model),
      source_listener_pos_changed_(false){
  hrtf_lookup_ = new HRTF(config);
  left_fft_filter_ = new FFTFilter(config_.block_size);
  right_fft_filter_ = new FFTFilter(config_.block_size);

  Init();
}

AudioRender::~AudioRender() {
  delete hrtf_lookup_;
  delete left_fft_filter_;
  delete right_fft_filter_;
}

void AudioRender::Init() {
  RenderReflections(room_model_->GetWalls());

  hftf_infos_.resize(acoustic_sources_.size());
}

void AudioRender::SetSourcePosition(const Point3D& source_pos) {
  source_pos_ = source_pos;
  source_listener_pos_changed_ = true;
}
void AudioRender::SetListenerPosition(const Point3D& listener_pos) {
  listener_pos_ = listener_pos;
  source_listener_pos_changed_ = true;
}

void AudioRender::RenderAudio(const std::vector<float>&input,
                                 std::vector<float>* output_left,
                              std::vector<float>* output_right) {
  assert(output_left != 0 && output_right != 0);

  bool hrtf_rendering_needed = false;

  if (room_model_->WallsHaveChanged() || source_listener_pos_changed_) {
    const std::vector<WallModel>& walls = room_model_->GetWalls();
    RenderReflections(walls);

    for (int i = 0; i < acoustic_sources_.size(); ++i) {
      const AcousticSource& source = acoustic_sources_[i];
      const HRTFInfo& source_hrtf = hrtf_lookup_->GetHRTFInfo(source.pos);
      if (source_hrtf!=hftf_infos_[i]) {
        hftf_infos_[i] = source_hrtf;
        hrtf_rendering_needed = true;
      }
    }
    source_listener_pos_changed_ = false;
  }

  if (hrtf_rendering_needed) {
    int hrtf_filter_len = hrtf_lookup_->GetFreqHRTFSize();

    std::vector<Complex> left_hftf(hrtf_filter_len, Complex(1.0f, 0.0f));
    std::vector<Complex> right_hftf(hrtf_filter_len, Complex(1.0f, 0.0f));

    for (int i = 0; i < hftf_infos_.size(); ++i) {
      const HRTFInfo& hrtf_info = hftf_infos_[i];
      const std::vector<Complex>& left_hrtf_filter = hrtf_lookup_->GetLeftEarFreqHRTF(
          hrtf_info);
      const std::vector<Complex>& right_hrtf_filter = hrtf_lookup_
          ->GetRightEarFreqHRTF(hrtf_info);

      float damping_factor = acoustic_sources_[i].damping;
      for (int i = 0; i < left_hrtf_filter.size(); ++i) {
        left_hftf[i] *= left_hrtf_filter[i] * damping_factor;
        right_hftf[i] *= left_hrtf_filter[i] * damping_factor;
      }
    }

    left_fft_filter_->SetFreqDomainKernel(left_hftf);
    right_fft_filter_->SetFreqDomainKernel(right_hftf);
  }

  left_fft_filter_->AddSignalBlock(input);
  left_fft_filter_->GetResult(output_left);

  right_fft_filter_->AddSignalBlock(input);
  left_fft_filter_->GetResult(output_right);

}

const std::vector<AcousticSource> AudioRender::RenderReflections(
    const std::vector<WallModel>& walls) const {
  std::vector<AcousticSource> rendered_reflections;

  int max_reflections = (1 + walls.size())
      * (walls.size() ^ (config_.reflection_order - 1));

  // Rebuild array of acoustic sources.
  rendered_reflections.clear();
  rendered_reflections.reserve(max_reflections);

  AcousticSource initial_source;
  initial_source.pos = source_pos_;
  initial_source.damping = 1.0f;
  initial_source.reflections = 0;
  initial_source.do_render = true;

  std::vector<AcousticSource> prev_sources;
  prev_sources.push_back(initial_source);
  rendered_reflections.push_back(initial_source);

  for (int ro = 0; ro < config_.reflection_order; ++ro) {
    std::vector<AcousticSource> new_sources;
    new_sources.reserve(prev_sources.size() * walls.size());

    for (int s = 0; s < prev_sources.size(); ++s) {
      for (int w = 0; w < walls.size(); ++w) {
        const WallModel& wall = walls[w];
        const AcousticSource& origin = prev_sources[s];

        if (origin.do_render==false) {
          AcousticSource new_source;
          new_source.do_render = false;
          new_sources.push_back(new_source);
          continue;
        }

        float wall_distance = origin.pos.x * wall.plane.a
            + origin.pos.y * wall.plane.b + origin.pos.z * wall.plane.c
            + wall.plane.d;

        AcousticSource new_source;
        if (wall_distance > 0) {
          float virtual_distance = -2.0f * wall_distance;
          new_source.pos.x = origin.pos.x + wall.plane.a * virtual_distance;
          new_source.pos.y = origin.pos.y + wall.plane.b * virtual_distance;
          new_source.pos.z = origin.pos.z + wall.plane.c * virtual_distance;
          new_source.reflections = origin.reflections + 1;
          new_source.damping = origin.damping * wall.damping;
          new_source.do_render = true;
          new_sources.push_back(new_source);
        } else {
          // Source is behind the wall -> do not render it
          new_source.do_render = false;
          new_sources.push_back(new_source);
        }
      }

    }
    rendered_reflections.insert(rendered_reflections.end(), new_sources.begin(),
                             new_sources.end());
    prev_sources.swap(new_sources);
  }

  return rendered_reflections;
}

