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

// def cubic_bezier2(ux, uy, ps):
//   """
//   u in [0, 1]
//   p_j the four relevant control points
//   """
//   assert ux >= 0.
//   assert ux <= 1.
//
//   assert uy >= 0.
//   assert uy <= 1.
//
//   ux2 = ux*ux
//   ux3 = ux2*ux
//
//   uy2 = uy*uy
//   uy3 = uy2*uy
//
//   cxs = [
//     (1. - ux)**3,
//     3*ux3 - 6*ux2 + 4,
//     -3.*ux3 + 3.*ux2 + 3.*ux + 1,
//     ux3,
//   ]
//   cys = [
//     (1. - uy)**3,
//     3*uy3 - 6*uy2 + 4,
//     -3.*uy3 + 3.*uy2 + 3.*uy + 1,
//     uy3,
//   ]
//   ret = 0.
//   for kx, cx in enumerate(cxs):
//     for ky, cy in enumerate(cys):
//       ret += cx*cy*ps[kx, ky]
//
//   return ret / 36.

template <int NU, int NV, int NX, int NY>
Eigen::Matrix<glm::dvec3, NU, NV>
CubicBSplineSurfaceyo(const Eigen::Matrix<glm::dvec3, NX+2*NExtra, NY+2*NExtra> &full_ps) {
  Eigen::Matrix<glm::dvec3, NU, NV> interpolated;
  for (int ku=0; ku<NU; ku++) {
    for (int kv=0; kv<NV; kv++) {
      const double sx = static_cast<double>(ku) / (static_cast<double>(NU) - 1);
      const double sy = static_cast<double>(kv) / (static_cast<double>(NV) - 1);
      const double tx = NExtra + 1 + sx * (NX - NExtra - 1); // t from 3 to n
      const double ty = NExtra + 1 + sy * (NU - NExtra - 1); // t from 3 to n

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

      //return cubic_bezier2(ux, uy, ps[interval_x-3:interval_x+1, interval_y-3: interval_y+1])
      assert(ux >= 0);
      assert(ux <= 1);
      assert(uy >= 0);
      assert(uy <= 1);

      const double ux2 = ux*ux;
      const double ux3 = ux2*ux;

      const double uy2 = uy*uy;
      const double uy3 = uy2*uy;

      std::array<double, 4> cxs = {
          std::pow(1. - ux, 3),
          3*ux3 - 6*ux2 + 4,
          -3.*ux3 + 3.*ux2 + 3.*ux + 1,
          ux3};

      std::array<double, 4> cys = {
          std::pow(1. - uy, 3),
          3*uy3 - 6*uy2 + 4,
          -3.*uy3 + 3.*uy2 + 3.*uy + 1,
          uy3};

      glm::dvec3 ret = {0, 0, 0};
      for (int kx=0; kx<4; kx++) {
        for (int ky=0; ky<4; ky++) {
          ret += cxs[kx]*cys[ky]*full_ps(interval_x - 3 + kx, interval_y - 3 + ky);
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
// def cubic_bspline2(sx, sy, ps):
//   """
//   s in [0, 1]
//   """
//
//   nx = ps.shape[0]
//   ny = ps.shape[1]
//   tx = 3 + sx * (nx - 3) # t from 3 to n
//   ty = 3 + sy * (ny - 3) # t from 3 to n
//
//   interval_x = int(np.floor(tx))
//   interval_y = int(np.floor(ty))
//   ux = tx - interval_x
//   uy = ty - interval_y
//
//   if interval_x == nx and ux == 0:
//     interval_x = nx - 1
//     ux = 1.
//
//   if interval_y == ny and uy == 0:
//     interval_y = ny - 1
//     uy = 1.
//
//   #print()
//   #print('tx:', tx, 'ty:', ty, 'interval_x:',interval_x, 'interval_y:', interval_y)
//   #print('ux:',ux,'uy:',uy)
//   assert interval_x >= 3
//   assert interval_y >= 3
//   assert interval_x < nx
//   assert interval_y < ny
//
//   return cubic_bezier2(ux, uy, ps[interval_x-3:interval_x+1, interval_y-3: interval_y+1])
//
// def cubic_clamped_bspline2(sx, sy, ps):
//   n_extra = 2
//   nx, ny, nps = ps.shape
//   clamped_ps = np.empty((nx+2*n_extra, ny+2*n_extra, nps))
//   clamped_ps.fill(np.nan)
//   clamped_ps[n_extra:-n_extra, n_extra:-n_extra, :] = ps
//
//   for k in range(n_extra):
//     # x edges
//     clamped_ps[   k, n_extra:-n_extra, :] = ps[ 0,  :, :]
//     clamped_ps[-1-k, n_extra:-n_extra, :] = ps[-1,  :, :]
//
//     # y edges
//     clamped_ps[n_extra:-n_extra,    k, :] = ps[ :,  0, :]
//     clamped_ps[n_extra:-n_extra, -1-k, :] = ps[ :, -1, :]
//
//     # corners
//     for j in range(n_extra):
//       clamped_ps[   k,    j, :] = ps[ 0,  0, :]
//       clamped_ps[   k, -1-j, :] = ps[ 0, -1, :]
//       clamped_ps[-1-k, -1-j, :] = ps[-1, -1, :]
//       clamped_ps[-1-k,    j, :] = ps[-1,  0, :]
//
//   assert not np.any(np.isnan(clamped_ps))
//   return cubic_bspline2(sx, sy, clamped_ps)


template <int NU, int NV, int NX, int NY>
Eigen::Matrix<glm::dvec3, NU, NV>
CubicBSplineSurface(const Eigen::Matrix<glm::dvec3, NX, NY> &ps) {
  const Eigen::Matrix<glm::dvec3, NX + 2*NExtra, NY + 2*NExtra> clamped_ps = PadSurface<NX, NY>(ps);
  return CubicBSplineSurfaceyo<NU, NV, NX, NY>(clamped_ps);
}

//
// def main():
//   ps = [
//     np.array([1., 0.]),
//     np.array([1., 1.]),
//     np.array([1.5, 1.]),
//     np.array([2., -1.]),
//     np.array([3., 0.]),
//     np.array([4., 1.]),
//     np.array([5., 0.]),
//   ]
//   ts = np.linspace(0., 1., 1000)
//   xys = np.array([cubic_clamped_bspline(t, ps) for t in ts])
//
//   plt.figure()
//   plt.plot([p[0] for p in ps], [p[1] for p in ps], 'r-.o')
//   plt.plot(xys[:, 0], xys[:, 1], 'b')
//
//   ps = np.array([
//     [(1.0, 0,  1.0), (1.0, 3,  1.0), (1.0, 4,  0.0), (1.0, 6,  1.0)],
//     [(1.5, 0,  1.0), (1.5, 3,  1.0), (1.5, 4,  1.0), (1.5, 6,  0.0)],
//     [(2.0, 0, -1.0), (2.0, 3, -2.0), (2.0, 4, -1.0), (2.0, 6, -1.0)],
//     [(3.0, 0,  0.0), (3.0, 3,  0.0), (3.0, 4,  1.0), (3.0, 7,  0.0)],
//     [(4.0, 0,  1.0), (4.0, 3,  4.0), (4.0, 4,  1.0), (4.0, 6,  1.0)],
//     [(5.0, 0,  0.0), (5.0, 3,  0.0), (5.0, 4,  0.0), (5.0, 6,  0.0)],
//   ])
//   us = np.linspace(0., 1., 50)
//   vs = np.linspace(0., 1., 51)
//
//   xyzs_interp = np.array([[cubic_clamped_bspline2(u, v, ps) for u in us] for v in vs])
//   xs = xyzs_interp[:, :, 0][:]
//   ys = xyzs_interp[:, :, 1][:]
//   zs = xyzs_interp[:, :, 2][:]
//
//   fig = plt.figure()#subplot(2, 1, 1)
//   ax = fig.add_subplot(111, projection='3d')
//   ax.scatter(xs, ys, zs)#, marker=m)
//
//   xs = ps[:, :, 0][:]
//   ys = ps[:, :, 1][:]
//   zs = ps[:, :, 2][:]
//   ax.scatter(xs, ys, zs)#, marker=m)
//   ax.legend(['interpolated', 'control points'])
//
//
//   plt.show()
