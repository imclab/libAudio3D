#include <cmath>
#include <iostream>
#include <vector>

#include "gtest/gtest.h"
#include "room_model.h"

using namespace std;

TEST(RoomModelTest, SingleWall) {

  RoomModel room(1);
  room.AddAcousticWall(-1.0, 0.0, 0.0, 2.0, 0.5);
  room.SetSourcePosition(1.0, 0.0, 0.0);

  const std::vector<AcousticSource>& acoustic_sources = room.RenderReflections();

  EXPECT_EQ(acoustic_sources.size(), 2);

  EXPECT_NEAR(acoustic_sources[0].pos.x, 1.0, 1e-6);
  EXPECT_NEAR(acoustic_sources[0].pos.y, 0.0, 1e-6);
  EXPECT_NEAR(acoustic_sources[0].pos.z, 0.0, 1e-6);

  EXPECT_NEAR(acoustic_sources[1].pos.x, 3.0, 1e-6);
  EXPECT_NEAR(acoustic_sources[1].pos.y, 0.0, 1e-6);
  EXPECT_NEAR(acoustic_sources[1].pos.z, 0.0, 1e-6);

}

TEST(RoomModelTest, Room) {
  RoomModel room(2);
  room.DefineBox(2.0, 2.0, 2.0, 1.0);
  room.SetSourcePosition(-2.0, 0.0, 0.0);

  const std::vector<AcousticSource>& acoustic_sources = room.RenderReflections();

  EXPECT_EQ(acoustic_sources.size(), 43);

  /*
  for (int i = 0; i < acoustic_sources.size(); ++i) {
    const AcousticSource& source = acoustic_sources[i];
    printf("Id:%d, x: %.4f, y: %.4f, z: %.4f %s\n", i, source.pos.x, source.pos.y,
           source.pos.z, source.do_render?"Y":"N");
  }
  */
}

