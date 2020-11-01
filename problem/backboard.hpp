#pragma once

#include <eigen3/Eigen/Dense>

template <int NX, int NY>
class Backboard {
public:
  Backboard() {
    for (int kx=0; kx<NX; kx++) {
      const double sx = 2 * kx / (double)(NX - 1) - 1; // -1 to 1
      for (int ky=0; ky<NY; ky++) {
        const double sy = 2 * ky / (double)(NY - 1) - 1; // -1 to 1

        const double z = sx - 1; // -2 to 0
        const double y = 2*sy; // -2 to 2
        const double x = sx*sx + 0.5 * sy*sy + 0.1 * sin(sx) * cos(2*sy);

        control_points_(kx, ky) = glm::dvec3(x, y, z);
      }
    }
  }

  template <int NU, int NV>
  Surface<NU, NV> Interpolate() {
    return ClampedCubicBSplineSurface<NU, NV, NX, NY>(control_points_);
  }

//private:
  Eigen::Matrix<glm::dvec3, NX, NY> control_points_;
};
