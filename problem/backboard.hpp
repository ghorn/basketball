#pragma once

#include <eigen3/Eigen/Dense>

template <int NX, int NY>
class Backboard {
public:
  Backboard() {
    control_points_ <<
      glm::dvec3(1.0,0, 1.0),glm::dvec3(1.0,3, 1.0),glm::dvec3(1.0,4, 0.0),glm::dvec3(1.0,6, 1.0),
      glm::dvec3(1.5,0, 1.0),glm::dvec3(1.5,3, 1.0),glm::dvec3(1.5,4, 1.0),glm::dvec3(1.5,6, 0.0),
      glm::dvec3(2.0,0,-1.0),glm::dvec3(2.0,3,-2.0),glm::dvec3(2.0,4,-1.0),glm::dvec3(2.0,6,-1.0),
      glm::dvec3(3.0,0, 0.0),glm::dvec3(3.0,3, 0.0),glm::dvec3(3.0,4, 1.0),glm::dvec3(3.0,7, 0.0),
      glm::dvec3(4.0,0, 1.0),glm::dvec3(4.0,3, 4.0),glm::dvec3(4.0,4, 1.0),glm::dvec3(4.0,6, 1.0),
      glm::dvec3(5.0,0, 0.0),glm::dvec3(5.0,3, 0.0),glm::dvec3(5.0,4, 0.0),glm::dvec3(5.0,6, 0.0);
  }

  template <int NU, int NV>
  Eigen::Matrix<glm::dvec3, 20, 30> Interpolate() {
    return ClampedCubicBSplineSurface<NU, NV, NX, NY>(control_points_);
  }

private:
  Eigen::Matrix<glm::dvec3, NX, NY> control_points_;
};
