#pragma once

#include <eigen3/Eigen/Dense>

#include "bspline.hpp"
#include "problem/hoop.hpp" // kRimHeight

template <int NX, int NY>
class Backboard {
public:
  static constexpr double kWidth = 1.83; // 6 feet
  static constexpr double kTall = 1.07; // 3.5 feet
  static constexpr double kBottomOfBackboard = Hoop::kRimHeight - 0.305; // 12 inches below rim

  Backboard() {
    for (int ku=0; ku<NX; ku++) {
      const double su = ku / (double)(NX - 1); // 0 to 1
      for (int kv=0; kv<NY; kv++) {
        const double sv = kv / (double)(NY - 1); // 0 to 1

        const double su_ = 2*su - 1; // -1 to 1
        const double sv_ = 2*sv - 1; // -1 to 1

        // 1.83 meters wide
        const double x = 0.5*kWidth * su_;
        // 1.07 meters tall
        const double z = -kBottomOfBackboard - (1 - sv) * kTall;

        const double y = 0.3*su_*su_ + 0.01*sv_*sv_ + 0.5*sin(3*su_)*cos(5*sv_);

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
