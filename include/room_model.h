#ifndef ROOM_MODEL_H_
#define ROOM_MODEL_H_

#include <vector>
#include "common.h"

struct WallModel {
  Plane plane;
  float damping;
};

class RoomModel {
 public:
  RoomModel();
  virtual ~RoomModel();

  void DefineBox(float width, float height, float depth, float damping);
  void AddAcousticWall(float a, float b, float c, float d, float damping);
  void Reset();

  void SetSourcePosition(const Vec3d_f& source_pos);
  void SetListenerPosition(const Vec3d_f& listener_pos);

  const Vec3d_f& GetSourcePosition() const;
  const Vec3d_f& GetListenerPosition() const;

  bool ModelHasChanged() const;
  void SetModelUnchanged();

  const std::vector<WallModel>& GetWalls() const;

 private:
  mutable bool model_has_changed_;
  std::vector<WallModel> walls_;

  Vec3d_f source_pos_;
  Vec3d_f listener_pos_;
};

#endif  // ROOM_MODEL_H_
