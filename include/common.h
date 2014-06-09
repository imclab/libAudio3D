#ifndef COMMON_H_
#define COMMON_H_

#include <cmath>
#include <complex>
#include <vector>
#include <assert.h>

static const float kSoundSpeedMeterPerSecond = 340.0;

static const int kDimensions = 3;

typedef std::complex<float> Complex;

union Point3D {
  float data[kDimensions];
  struct {
    float x;
    float y;
    float z;
  };
};

Point3D operator+(const Point3D& point_a, const Point3D& point_b) ;

Point3D operator-(const Point3D& point_a, const Point3D& point_b);

float PointDot(const Point3D& point_a, const Point3D& point_b);

float VecLen(const Point3D& point_a);

struct AcousticSource {
  Point3D pos;
  float damping;
  int reflections;
  bool do_render;
};

struct HRTFInfo {
  HRTFInfo();
  int distance_index;
  int hrtf_index;
  bool left_right_swap;
};

bool operator==(const HRTFInfo& hrtf_a, const HRTFInfo& hrtf_b);
bool operator!=(const HRTFInfo& hrtf_a, const HRTFInfo& hrtf_b);

union Plane {
  float norm[3];  // plane normal
  float data[4];  // plane equation
  struct {
    float a;
    float b;
    float c;
    float d;
  };
};

struct WallModel {
  Plane plane;
  float damping;
};



#endif COMMON_H_
