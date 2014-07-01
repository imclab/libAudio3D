#include <assert.h>
#include "room_model.h"

RoomModel::RoomModel()
    : model_has_changed_(false) {
}

RoomModel::~RoomModel() {

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

  model_has_changed_ = true;
}

void RoomModel::AddAcousticWall(float a, float b, float c, float d,
                                float damping) {
  float l2_normal = sqrt(a * a + b * b + c * c);
  assert( fabs(l2_normal - 1.0f) < 1e-5 && "Normal not normalized to 1.0");

  WallModel new_wall;
  new_wall.plane.a = a;
  new_wall.plane.b = b;
  new_wall.plane.c = c;
  new_wall.plane.d = d;
  new_wall.damping = damping;
  walls_.push_back(new_wall);

  model_has_changed_ = true;
}

void RoomModel::Reset() {
  walls_.clear();
  model_has_changed_ = true;
}

bool RoomModel::ModelHasChanged() const {
  return model_has_changed_;
}

void RoomModel::SetModelUnchanged() {
  model_has_changed_ = false;
}

const std::vector<WallModel>& RoomModel::GetWalls() const {
  return walls_;
}

void RoomModel::SetSourcePosition(const Vec3d_f& source_pos) {
  source_pos_ = source_pos;
  model_has_changed_ = true;
}
void RoomModel::SetListenerPosition(const Vec3d_f& listener_pos) {
  listener_pos_ = listener_pos;
  model_has_changed_ = true;
}

const Vec3d_f& RoomModel::GetSourcePosition() const {
  return source_pos_;
}
const Vec3d_f& RoomModel::GetListenerPosition() const {
  return listener_pos_;
}
