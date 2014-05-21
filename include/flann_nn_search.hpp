#ifndef FLANN_NN_SEARCH_H_
#define FLANN_NN_SEARCH_H_

#include <cmath>
#include <assert.h>
#include <flann/flann.hpp>
#include "common.h"

class FLANNNeighborSearch {
 public:
  FLANNNeighborSearch()
      : flann_index_(0) {
  }
  virtual ~FLANNNeighborSearch() {
    if (flann_index_ != 0) {
      delete flann_index_;
    }
  }

  void BuildIndex() {
    assert(!flann_index_ && "FLANN index already built");

    hrtf_pos_on_unit_sphere_mat_ = flann::Matrix<float>(
        &hrtf_pos_on_unit_sphere_[0].data[0], hrtf_pos_on_unit_sphere_.size(),
        kDimensions);

    flann_index_ = new flann::Index<flann::L2<float> >(
        hrtf_pos_on_unit_sphere_mat_, flann::KDTreeIndexParams());
    flann_index_->buildIndex();
  }

  // Front center is at (0, 0)
  // elevation_deg range from -90 to 90, azimuth_deg range from -180 to 180
  void AddHRTFDirection(float elevation_deg, float azimuth_deg, int index) {
    Point3D carthesian_point_on_unit_sphere;
    GetPointOnUnitSphere(elevation_deg + 90.0f, azimuth_deg,
                         &carthesian_point_on_unit_sphere);

    hrtf_pos_on_unit_sphere_.push_back(carthesian_point_on_unit_sphere);
    hrtf_indices_.push_back(index);
  }

  // Front center is at (0, 0)
  // elevation_deg range from -90 to 90, azimuth_deg range from -180 to 180
  int FindNearestHRTF(float elevation_deg, float azimuth_deg) {
    assert(flann_index_ && "FLANN index missing");
    // Flann query
    Point3D carthesian_point_on_unit_sphere;
    GetPointOnUnitSphere(elevation_deg + 90.0f, azimuth_deg,
                         &carthesian_point_on_unit_sphere);
    flann::Matrix<float> query_mat(&carthesian_point_on_unit_sphere.data[0], 1,
                                   kDimensions);

    // Prepare result data structures.
    int result_index;
    flann::Matrix<int> index_mat(&result_index, 1, 1);
    float result_distance;
    ::flann::Matrix<float> distances_mat(&result_distance, 1, 1);

    flann_index_->knnSearch(query_mat, index_mat, distances_mat, 1,
                            flann::SearchParams());

    assert(
        result_index >= 0
            && result_index < static_cast<int>(hrtf_indices_.size()));
    return hrtf_indices_[result_index];
  }

 private:
  void GetPointOnUnitSphere(float elevation_deg, float azimuth_deg,
                            Point3D* carthesian_point) {
    assert(carthesian_point);

    float elevation_rad = elevation_deg * M_PI / 180.0;
    float azimuth_rad = azimuth_deg * M_PI / 180.0;

    carthesian_point->x = cos(azimuth_rad) * sin(elevation_rad);
    carthesian_point->y = sin(azimuth_rad) * sin(elevation_rad);
    carthesian_point->z = cos(elevation_rad);
  }

  std::vector<Point3D> hrtf_pos_on_unit_sphere_;
  flann::Matrix<float> hrtf_pos_on_unit_sphere_mat_;
  std::vector<int> hrtf_indices_;
  flann::Index<flann::L2<float> >* flann_index_;

};

#endif  // FLANN_NN_SEARCH_H_
