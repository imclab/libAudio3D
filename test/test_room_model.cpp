#include <cmath>
#include <iostream>
#include <vector>

#include "gtest/gtest.h"
#include "room_model.h"

using namespace std;

TEST(RoomModelTest, SingleWall) {

  RoomModel room;
  room.AddWall(-1.0, 0.0, 0.0, 2.0, 0.5);
  room.SetSourcePosition(0.0, 0.0, 0.0);

  std::vector<AcousticSource> acoustic_sources;
  room.RenderReflections(1,&acoustic_sources);

  for (int i = 0; i < acoustic_sources.size(); ++i) {
    const AcousticSource& source = acoustic_sources[i];
    printf("Id:%d, x: %.4f, y: %.4f, z: %.4f\n", i, source.pos.x, source.pos.y,
           source.pos.z);
  }

}


