#include <assert.h>
#include <cmath>
#include "room_model.h"

RoomModel::RoomModel(int reflection_order)
    : source_pos_changed_(true),
      reflection_order_(reflection_order) {
  source_pos_.x = source_pos_.y = source_pos_.z = 0.0f;
}

RoomModel::~RoomModel() {

}

void RoomModel::SetSourcePosition(float x, float y, float z) {
  Point3D source_pos;
  source_pos.x = x;
  source_pos.y = y;
  source_pos.z = z;
  SetSourcePosition(source_pos);
}

void RoomModel::SetSourcePosition(const Point3D& source_pos) {
  if (source_pos_.x != source_pos.x || source_pos_.y != source_pos.y
      || source_pos_.z != source_pos.z) {
    source_pos_changed_=true;
    source_pos_ = source_pos;
  }
}

void RoomModel::DefineBox(float width, float height, float depth,
                          float damping) {
  float half_width = 0.5 * width;
  AddAcousticWall(1.0, 0.0, 0.0, half_width, damping);
  AddAcousticWall(-1.0, 0.0, 0.0, half_width, damping);

  float half_height = 0.5 * height;
  AddAcousticWall(0.0, 1.0, 0.0, half_height, damping);
  AddAcousticWall(0.0, -1.0, 0.0, half_height, damping);

  float half_depth = 0.5 * depth;
  AddAcousticWall(0.0, 0.0, 1.0, half_depth, damping);
  AddAcousticWall(0.0, 0.0, -1.0, half_depth, damping);
}

void RoomModel::AddAcousticWall(float a, float b, float c, float d,
                                    float damping) {
  assert(
      fabs(sqrt(a * a + b * b + c * c) - 1.0) < 1e-5 && "Plane normal must be normalized to 1.0");
  assert(
      acoustic_sources_.size()==0 && "Wall configuration cannot be changed after first rendering");

  WallModel new_wall;
  new_wall.plane.a = a;
  new_wall.plane.b = b;
  new_wall.plane.c = c;
  new_wall.plane.d = d;
  new_wall.damping = damping;
  walls_.push_back(new_wall);
}

const std::vector<AcousticSource>& RoomModel::RenderReflections() {
  if (source_pos_changed_==false) {
    return acoustic_sources_;
  }
  source_pos_changed_ = false;
  acoustic_sources_.clear();

  AcousticSource initial_source;
  initial_source.pos = source_pos_;
  initial_source.damping = 1.0f;
  initial_source.reflections = 0;
  initial_source.do_render = true;

  std::vector<AcousticSource> prev_sources;
  prev_sources.push_back(initial_source);
  acoustic_sources_.push_back(initial_source);

  for (int ro = 0; ro < reflection_order_; ++ro) {
    std::vector<AcousticSource> new_sources;
    new_sources.reserve(prev_sources.size() * walls_.size());

    for (int s = 0; s < prev_sources.size(); ++s) {
      for (int w = 0; w < walls_.size(); ++w) {
        const WallModel& wall = walls_[w];
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
    acoustic_sources_.insert(acoustic_sources_.end(), new_sources.begin(),
                             new_sources.end());
    prev_sources.swap(new_sources);
  }

  return acoustic_sources_;
}
