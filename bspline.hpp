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

template <int NU, int NV>
struct Surface {
  Eigen::Matrix<glm::dvec3, NU, NV> position;
  Eigen::Matrix<glm::dvec3, NU, NV> tangent_u;
  Eigen::Matrix<glm::dvec3, NU, NV> tangent_v;
  Eigen::Matrix<glm::dvec3, NU, NV> normal;
};

template <int NU, int NV, int NX, int NY>
Surface<NU, NV>
CubicBSplineSurface(const Eigen::Matrix<glm::dvec3, NX, NY> &ps) {
  Surface<NU, NV> interpolated;
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

      const std::array<double, 4> deriv_cxs = {
          -3 * (1. - ux)*(1. - ux),
          9*ux2 - 12*ux,
          -9.*ux2 + 6.*ux + 3.,
          3*ux2};
      const std::array<double, 4> deriv_cys = {
          -3 * (1. - uy)*(1. - uy),
          9*uy2 - 12*uy,
          -9.*uy2 + 6.*uy + 3.,
          3*uy2};

      glm::dvec3 position = {0, 0, 0};
      glm::dvec3 tangent_u = {0, 0, 0};
      glm::dvec3 tangent_v = {0, 0, 0};
      for (int kx=0; kx<4; kx++) {
        for (int ky=0; ky<4; ky++) {
          const glm::dvec3 &p = ps(interval_x - 3 + kx, interval_y - 3 + ky);
          position  +=       cxs[kx]*      cys[ky]*p;
          tangent_u += deriv_cxs[kx]*      cys[ky]*p;
          tangent_v +=       cxs[kx]*deriv_cys[ky]*p;
        }
      }
      position.x /= 36;
      position.y /= 36;
      position.z /= 36;
      tangent_u.x /= 36;
      tangent_u.y /= 36;
      tangent_u.z /= 36;
      tangent_v.x /= 36;
      tangent_v.y /= 36;
      tangent_v.z /= 36;
      interpolated.position(ku, kv) = position;
      interpolated.tangent_u(ku, kv) = tangent_u;
      interpolated.tangent_v(ku, kv) = tangent_v;
      interpolated.normal(ku, kv) = glm::normalize(glm::cross(tangent_u, tangent_v));
    }
  }

  return interpolated;
}


template <int NU, int NV, int NX, int NY>
Surface<NU, NV>
ClampedCubicBSplineSurface(const Eigen::Matrix<glm::dvec3, NX, NY> &ps) {
  const Eigen::Matrix<glm::dvec3, NX + 2*NExtra, NY + 2*NExtra> clamped_ps = PadSurface<NX, NY>(ps);
  return CubicBSplineSurface<NU, NV, NX + 2*NExtra, NY + 2*NExtra>(clamped_ps);
}
