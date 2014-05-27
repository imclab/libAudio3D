#ifndef ROOM_MODEL_H_
#define ROOM_MODEL_H_

#include <vector>
#include "common.h"

class RoomModel {
 public:
  RoomModel(int reflection_order);
  virtual ~RoomModel();

  void SetSourcePosition(float x, float y, float z);
  void SetSourcePosition(const Point3D& source_pos);

  void DefineBox(float width, float height, float depth, float damping);
  void AddAcousticWall(float a, float b, float c, float d, float damping);

  const std::vector<AcousticSource>& RenderReflections();
 private:
  Point3D source_pos_;
  bool source_pos_changed_;
  int reflection_order_;
  std::vector<WallModel> walls_;

  std::vector<AcousticSource> acoustic_sources_;
};

#endif  // ROOM_MODEL_H_
