#ifndef COMMON_H_
#define COMMON_H_

static const int kDimensions = 3;

union Point3D {
  float data[kDimensions];
  struct {
    float x;
    float y;
    float z;
  };
};

struct AcousticSource {
  Point3D pos;
  float damping;
  int reflections;
};

union WallModel{
  float data[4];  // plane equation
  struct {
    float a;
    float b;
    float c;
    float d;
  };
  float damping;
};

#endif COMMON_H_
