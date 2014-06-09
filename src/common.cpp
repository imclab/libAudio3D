#include "common.h"
#include <assert.h>

Point3D operator+(const Point3D& point_a, const Point3D& point_b) {
  Point3D ret;
  ret.x = point_a.x + point_b.x;
  ret.y = point_a.y + point_b.y;
  ret.z = point_a.z + point_b.z;
  return ret;
}

Point3D operator-(const Point3D& point_a, const Point3D& point_b) {
  Point3D ret;
  ret.x = point_a.x - point_b.x;
  ret.y = point_a.y - point_b.y;
  ret.z = point_a.z - point_b.z;
  return ret;
}

std::ostream& operator<<(std::ostream& stream, const Point3D& point) {
  stream << "x: " << point.x << ", y: " << point.y << ", z: " << point.z;
  return stream;
}

float PointDot(const Point3D& point_a, const Point3D& point_b) {
  return point_a.x * point_b.x + point_a.y * point_b.y + point_a.z * point_b.z;
}

float VecLen(const Point3D& point_a) {
  return sqrt(PointDot(point_a, point_a));
}

HRTFInfo::HRTFInfo()
    : distance_index(-1),
      hrtf_index(-1),
      left_right_swap(false) {
}

bool operator==(const HRTFInfo& hrtf_a, const HRTFInfo& hrtf_b) {
  return hrtf_a.distance_index == hrtf_b.distance_index
      && hrtf_a.hrtf_index == hrtf_b.hrtf_index
      && hrtf_a.left_right_swap == hrtf_b.left_right_swap;
}

bool operator!=(const HRTFInfo& hrtf_a, const HRTFInfo& hrtf_b) {
  return !operator==(hrtf_a, hrtf_b);
}

