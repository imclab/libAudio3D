#ifndef ROOM_MODEL_H_
#define ROOM_MODEL_H_

#include <vector>
#include "common.h"

class RoomModel {
 public:
  RoomModel();
  virtual ~RoomModel();

  void SetSourcePosition(float x, float y, float z);
  void SetSourcePosition(const Point3D& source_pos);

  void DefineBox(float width, float height, float , float damping);
  void AddWall(float a, float b, float c, float d, float damping);

  void RenderReflections(int reflection_order,
                          std::vector<AcousticSource>* acoustic_sources);
 private:
  Point3D source_pos_;
  std::vector<WallModel> walls_;

};

#endif  // ROOM_MODEL_H_
