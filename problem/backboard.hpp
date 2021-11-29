#pragma once

#include <cmath>              // for cos, sin
#include <eigen3/Eigen/Dense>  // for Matrix
#include <glm/glm.hpp>         // for dvec3

#include "bspline.hpp"         // for ClampedCubicBSplineSurface, Surface
#include "problem/hoop.hpp"    // for Hoop, Hoop::kRimHeight

template <int NX, int NY>
class Backboard {
public:
  static constexpr double kWidth = 1.83; // 6 feet
  static constexpr double kTall = 1.07; // 3.5 feet
  //static constexpr double kBottomOfBackboard = Hoop::kRimHeight - 0.305; // 12 inches below rim
  static constexpr double kBottomOfBackboard = Hoop::kRimHeight + 0.305; // 12 inches above rim

  static Eigen::Matrix<glm::dvec3, NX, NY> Initialize() {
    Eigen::Matrix<glm::dvec3, NX, NY> control_points;

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

        control_points(ku, kv) = glm::dvec3(x, y, z);
      }
    }
    return control_points;
  }

  static Eigen::Matrix<double, NX, NY> FromControlPoints(Eigen::Matrix<glm::dvec3, NX, NY> control_points) {
    Eigen::Matrix<double, NX, NY> dvs;
    for (int kx=0; kx<NX; kx++) {
      for (int ky=0; ky<NY; ky++) {
        dvs(kx, ky) = control_points(kx, ky).y;
      }
    }
    return dvs;
  }

  static Eigen::Matrix<glm::dvec3, NX, NY> ToControlPoints(const Eigen::Matrix<double, NX, NY> &dvs) {
    Eigen::Matrix<glm::dvec3, NX, NY> control_points = Initialize(); // Inefficient
    for (int kx=0; kx<NX; kx++) {
      for (int ky=0; ky<NY; ky++) {
        control_points(kx, ky).y = dvs(kx, ky);
      }
    }
    return control_points;
  }

  template <int NU, int NV>
  static Surface<NU, NV> Interpolate(const Eigen::Matrix<glm::dvec3, NX, NY> &control_points) {
    return ClampedCubicBSplineSurface<NU, NV, NX, NY>(control_points);
  }
};
