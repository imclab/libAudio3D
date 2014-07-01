#ifndef COMMON_H_
#define COMMON_H_

#include <cmath>
#include <complex>
#include <vector>
#include <iostream>
#include <assert.h>

static const float kSoundSpeedMeterPerSecond = 340.0;

static const int kDimensions = 3;

typedef std::complex<float> Complex;

union Vec3d_f {
  float data[kDimensions];
  struct {
    float x;
    float y;
    float z;
  };
};

Vec3d_f operator+(const Vec3d_f& point_a, const Vec3d_f& point_b) ;

Vec3d_f operator-(const Vec3d_f& point_a, const Vec3d_f& point_b);

std::ostream& operator<<(std::ostream& stream, const Vec3d_f& point);

float Vec3dDot(const Vec3d_f& point_a, const Vec3d_f& point_b);

float Vec3dNorm(const Vec3d_f& point_a);

struct AcousticSource {
  Vec3d_f pos;
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


#endif COMMON_H_
