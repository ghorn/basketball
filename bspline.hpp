#pragma once

#include <eigen3/Eigen/Dense>
#include <glm/glm.hpp>

#define NExtra 2

template <int NX, int NY>
Eigen::Matrix<glm::dvec3, NX + 2*NExtra, NY + 2*NExtra>
PadSurface(const Eigen::Matrix<glm::dvec3, NX, NY> &ps) {
  //clamped_ps[n_extra:-n_extra, n_extra:-n_extra, :] = ps
  Eigen::Matrix<glm::dvec3, NX + 2*NExtra, NY + 2*NExtra> clamped_ps;
  //clamped_ps.block<NX, NY>(NExtra, NExtra) = ps;
  for (int kx=0; kx<NX; kx++) {
    for (int ky=0; ky<NY; ky++) {
      clamped_ps(kx+NExtra, ky+NExtra) = ps(kx, ky);
    }
  }

  // bottom/top edges
  for (int kx=0; kx < NExtra; kx++) {
    for (int ky=0; ky < NY; ky++) {
      // top edge
      clamped_ps(kx, ky + NExtra) = ps(0, ky);
      // bottom edge
      clamped_ps(NX + 2*NExtra - 1 - kx, ky + NExtra) = ps(NX - 1, ky);
    }
  }

  // left/right edges
  for (int ky=0; ky < NExtra; ky++) {
    for (int kx=0; kx < NX; kx++) {
      // left edge
      clamped_ps(kx + NExtra, ky) = ps(kx, 0);
      // right edge
      clamped_ps(kx + NExtra, NY + 2*NExtra - 1 - ky) = ps(kx, NY-1);
    }
  }

  // corners
  for (int kx=0; kx < NExtra; kx++) {
    for (int ky=0; ky < NExtra; ky++) {
      // upper left
      clamped_ps(kx, ky) = ps(0, 0);
      // upper right
      clamped_ps(kx, NY + 2*NExtra - 1 - ky) = ps(0, NY - 1);
      // bottom left
      clamped_ps(NX + 2*NExtra - 1 - kx, ky) = ps(NX - 1, 0);
      // bottom right
      clamped_ps(NX + 2*NExtra - 1 - kx,  NY + 2*NExtra - 1 - ky) = ps(NX - 1, NY - 1);
    }
  }
  return clamped_ps;
}

static inline double Cubed(const double x) {
  return x*x*x;
}

template <int NU, int NV, int NX, int NY>
Eigen::Matrix<glm::dvec3, NU, NV>
CubicBSplineSurface(const Eigen::Matrix<glm::dvec3, NX, NY> &ps) {
  Eigen::Matrix<glm::dvec3, NU, NV> interpolated;
  for (int ku=0; ku<NU; ku++) {
    for (int kv=0; kv<NV; kv++) {
      const double sx = static_cast<double>(ku) / (static_cast<double>(NU) - 1);
      const double sy = static_cast<double>(kv) / (static_cast<double>(NV) - 1);
      const double tx = 3 + sx * (NX - 3); // t from 3 to n
      const double ty = 3 + sy * (NY - 3); // t from 3 to n

      int interval_x = static_cast<int>(std::floor(tx));
      int interval_y = static_cast<int>(std::floor(ty));
      double ux = tx - static_cast<double>(interval_x);
      double uy = ty - static_cast<double>(interval_y);

      if (interval_x == NX && ux == 0) {
        interval_x = NX - 1;
        ux = 1;
      }

      if (interval_y == NY && uy == 0) {
        interval_y = NY - 1;
        uy = 1;
      }

      assert(ux >= 0);
      assert(ux <= 1);
      assert(uy >= 0);
      assert(uy <= 1);

      const double ux2 = ux*ux;
      const double ux3 = ux2*ux;

      const double uy2 = uy*uy;
      const double uy3 = uy2*uy;

      const std::array<double, 4> cxs = {
          Cubed(1. - ux),
          3*ux3 - 6*ux2 + 4,
          -3.*ux3 + 3.*ux2 + 3.*ux + 1,
          ux3};

      const std::array<double, 4> cys = {
          Cubed(1. - uy),
          3*uy3 - 6*uy2 + 4,
          -3.*uy3 + 3.*uy2 + 3.*uy + 1,
          uy3};

      glm::dvec3 ret = {0, 0, 0};
      for (int kx=0; kx<4; kx++) {
        for (int ky=0; ky<4; ky++) {
          ret += cxs[kx]*cys[ky]*ps(interval_x - 3 + kx, interval_y - 3 + ky);
        }
      }
      ret.x /= 36;
      ret.y /= 36;
      ret.z /= 36;
      interpolated(ku, kv) = ret;
    }
  }

  return interpolated;
}


template <int NU, int NV, int NX, int NY>
Eigen::Matrix<glm::dvec3, NU, NV>
ClampedCubicBSplineSurface(const Eigen::Matrix<glm::dvec3, NX, NY> &ps) {
  const Eigen::Matrix<glm::dvec3, NX + 2*NExtra, NY + 2*NExtra> clamped_ps = PadSurface<NX, NY>(ps);
  return CubicBSplineSurface<NU, NV, NX + 2*NExtra, NY + 2*NExtra>(clamped_ps);
}
