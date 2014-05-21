#include <assert.h>
#include <cmath>
#include "room_model.h"

RoomModel::RoomModel() {
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
  source_pos_ = source_pos;
}

void RoomModel::DefineBox(float width, float height, float depth,
                          float damping) {
  AddWall(1.0, 0.0, 0.0, -0.5 * width, damping);
  AddWall(-1.0, 0.0, 0.0, 0.5 * width, damping);

  AddWall(0.0, 1.0, 0.0, -0.5 * height, damping);
  AddWall(0.0, -1.0, 0.0, 0.5 * height, damping);

  AddWall(0.0, 0.0, 1.0, -0.5 * depth, damping);
  AddWall(0.0, 0.0, -1.0, 0.5 * depth, damping);
}

void RoomModel::AddWall(float a, float b, float c, float d, float damping) {
  assert(
      fabs(sqrt(a * a + b * b + c * c) - 1.0) < 1e-5
          && "Plane normal must be normalized to 1.0");
  WallModel new_wall;
  new_wall.a = a;
  new_wall.b = b;
  new_wall.c = c;
  new_wall.d = d;
  new_wall.damping = damping;
  walls_.push_back(new_wall);
}

void RoomModel::RenderReflections(
    int reflection_order, std::vector<AcousticSource>* acoustic_sources) {
  assert(acoustic_sources);
  acoustic_sources->clear();

  AcousticSource initial_source;
  initial_source.pos = source_pos_;
  initial_source.damping = 1.0f;
  initial_source.reflections = 0;
  acoustic_sources->push_back(initial_source);

  for (int ro = 0; ro < reflection_order; ++ro) {
    std::vector<AcousticSource> new_rays;
    new_rays.reserve(acoustic_sources->size() * walls_.size());

    for (int s = 0; s < acoustic_sources->size(); ++s) {
      for (int w = 0; w < walls_.size(); ++w) {
        const WallModel& wall = walls_[w];
        const AcousticSource& origin = (*acoustic_sources)[s];

        float wall_distance = origin.pos.x * wall.a + origin.pos.y * wall.b
            + origin.pos.z * wall.c + wall.d;
printf("%d Distance: %.4f\n", w, wall_distance);
        if (wall_distance > 0) {
          AcousticSource new_source;
          float virtual_distance = 2.0f * wall_distance;
printf("%d VDistance: %.4f\n", w, virtual_distance);
          new_source.pos.x = origin.pos.x + wall.a * virtual_distance;
          new_source.pos.y = origin.pos.y + wall.b * virtual_distance;
          new_source.pos.z = origin.pos.z + wall.c * virtual_distance;
          new_source.reflections = origin.reflections + 1;
          new_source.damping = origin.damping * wall.damping;
          new_rays.push_back(new_source);
        }
      }
    }
    acoustic_sources->insert(acoustic_sources->end(), new_rays.begin(), new_rays.end());
  }
}

