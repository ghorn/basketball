#pragma once

#include <eigen3/Eigen/Dense>

#include "bspline.hpp"

template <int NX, int NY>
class Backboard {
public:
  Backboard() {
    for (int ku=0; ku<NX; ku++) {
      const double su = 2 * ku / (double)(NX - 1) - 1; // -1 to 1
      for (int kv=0; kv<NY; kv++) {
        const double sv = 2 * kv / (double)(NY - 1) - 1; // -1 to 1

        const double x = 2 * su; // -2 to 2
        const double z = sv - 2; // -1 to -3
        const double y = 0.3*su*su + 0.01*sv*sv + 0.5*sin(3*su)*cos(5*sv);

        control_points_(ku, kv) = glm::dvec3(x, y, z);
      }
    }
  }

  template <int NU, int NV>
  Surface<NU, NV> Interpolate() const {
    return ClampedCubicBSplineSurface<NU, NV, NX, NY>(control_points_);
  }

//private:
  Eigen::Matrix<glm::dvec3, NX, NY> control_points_;
};
