#ifndef ROOM_MODEL_H_
#define ROOM_MODEL_H_

#include <vector>
#include "common.h"

class RoomModel {
 public:
  RoomModel();
  virtual ~RoomModel();

  void DefineBox(float width, float height, float depth, float damping);
  void AddAcousticWall(float a, float b, float c, float d, float damping);
  void Reset();

  bool WallsHaveChanged() const;
  const std::vector<WallModel>& GetWalls() const;

 private:
  mutable bool walls_have_changed_;
  std::vector<WallModel> walls_;
};

#endif  // ROOM_MODEL_H_
