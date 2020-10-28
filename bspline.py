#!/usr/bin/env python3

import argparse
import numpy as np
import time
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

def cubic_bezier(u, p0, p1, p2, p3):
  """
  u in [0, 1]
  p_j the four relevant control points
  """
  assert u >= 0.
  assert u <= 1.

  u2 = u*u
  u3 = u2*u

  ret  = (1. - u)**3 * p0
  ret += (3*u3 - 6*u2 + 4) * p1
  ret += (-3.*u3 + 3.*u2 + 3.*u + 1) * p2
  ret += u3 * p3

  return ret / 6.


def cubic_bspline(s, ps):
  """
  s in [0, 1]
  """

  n = len(ps)
  t = 3 + s * (n - 3) # t from 3 to n

  interval = int(np.floor(t))
  u = t - interval

  if interval == n and u == 0:
    interval = n - 1
    u = 1.

  print()
  #print('n:',n)
  print('t:', t, 'interval:',interval)
  print('u:',u)
  assert interval >= 3
  assert interval < n

  return cubic_bezier(u, ps[interval-3], ps[interval-2], ps[interval-1], ps[interval])

def cubic_bezier2(ux, uy, ps):
  """
  u in [0, 1]
  p_j the four relevant control points
  """
  assert ux >= 0.
  assert ux <= 1.

  assert uy >= 0.
  assert uy <= 1.

  ux2 = ux*ux
  ux3 = ux2*ux

  uy2 = uy*uy
  uy3 = uy2*uy

  cxs = [
    (1. - ux)**3,
    3*ux3 - 6*ux2 + 4,
    -3.*ux3 + 3.*ux2 + 3.*ux + 1,
    ux3,
  ]
  cys = [
    (1. - uy)**3,
    3*uy3 - 6*uy2 + 4,
    -3.*uy3 + 3.*uy2 + 3.*uy + 1,
    uy3,
  ]
  ret = 0.
  for kx, cx in enumerate(cxs):
    for ky, cy in enumerate(cys):
      ret += cx*cy*ps[kx, ky]

  return ret / 36.

def cubic_bspline2(sx, sy, ps):
  """
  s in [0, 1]
  """
  #assert 2 == ps.dim
  print(ps.shape)
  assert ps.shape == (6+4, 4+4, 3) # REMOVE!!!!!!!!! ONE TIME SANITY CHECK!

  nx = ps.shape[0]
  ny = ps.shape[1]
  tx = 3 + sx * (nx - 3) # t from 3 to n
  ty = 3 + sy * (ny - 3) # t from 3 to n

  interval_x = int(np.floor(tx))
  interval_y = int(np.floor(ty))
  ux = tx - interval_x
  uy = ty - interval_y

  if interval_x == nx and ux == 0:
    interval_x = nx - 1
    ux = 1.

  if interval_y == ny and uy == 0:
    interval_y = ny - 1
    uy = 1.

  print()
  #print('n:',n)
  print('tx:', tx, 'ty:', ty, 'interval_x:',interval_x, 'interval_y:', interval_y)
  print('ux:',ux,'uy:',uy)
  assert interval_x >= 3
  assert interval_y >= 3
  assert interval_x < nx
  assert interval_y < ny

  return cubic_bezier2(ux, uy, ps[interval_x-3:interval_x+1, interval_y-3: interval_y+1])

def cubic_clamped_bspline(s, ps):
  n_extra = 2
  return cubic_bspline(s, n_extra*[ps[0]] + ps + n_extra*[ps[-1]])

def cubic_clamped_bspline2(sx, sy, ps):
  nx, ny, nps = ps.shape
  clamped_ps = np.empty((nx+4, ny+4, nps))
  clamped_ps.fill(np.nan)
  clamped_ps[2:-2, 2:-2, :] = ps

  # x edges
  clamped_ps[   0, 2:-2, :] = ps[ 0,  :, :]
  clamped_ps[   1, 2:-2, :] = ps[ 0,  :, :]
  clamped_ps[  -2, 2:-2, :] = ps[-1,  :, :]
  clamped_ps[  -1, 2:-2, :] = ps[-1,  :, :]

  # y edges
  clamped_ps[2:-2,    0, :] = ps[ :,  0, :]
  clamped_ps[2:-2,    1, :] = ps[ :,  0, :]
  clamped_ps[2:-2,   -2, :] = ps[ :, -1, :]
  clamped_ps[2:-2,   -1, :] = ps[ :, -1, :]
  # corners
  clamped_ps[0, 0, :] = ps[0, 0, :]
  clamped_ps[1, 0, :] = ps[0, 0, :]
  clamped_ps[0, 1, :] = ps[0, 0, :]
  clamped_ps[1, 1, :] = ps[0, 0, :]

  clamped_ps[0, -1, :] = ps[0, -1, :]
  clamped_ps[1, -1, :] = ps[0, -1, :]
  clamped_ps[0, -2, :] = ps[0, -1, :]
  clamped_ps[1, -2, :] = ps[0, -1, :]

  clamped_ps[-1, -1, :] = ps[-1, -1, :]
  clamped_ps[-2, -1, :] = ps[-1, -1, :]
  clamped_ps[-1, -2, :] = ps[-1, -1, :]
  clamped_ps[-2, -2, :] = ps[-1, -1, :]

  clamped_ps[-1, 0, :] = ps[-1, 0, :]
  clamped_ps[-2, 0, :] = ps[-1, 0, :]
  clamped_ps[-1, 1, :] = ps[-1, 0, :]
  clamped_ps[-2, 1, :] = ps[-1, 0, :]

  assert not np.any(np.isnan(clamped_ps))
  return cubic_bspline2(sx, sy, clamped_ps)

def main():
  ps = [
    np.array([1., 0.]),
    np.array([1., 1.]),
    np.array([1.5, 1.]),
    np.array([2., -1.]),
    np.array([3., 0.]),
    np.array([4., 1.]),
    np.array([5., 0.]),
  ]
  ts = np.linspace(0., 1., 1000)
  xys = np.array([cubic_clamped_bspline(t, ps) for t in ts])
  print(xys.shape)

  plt.figure()
  plt.plot([p[0] for p in ps], [p[1] for p in ps], 'r-.o')
  plt.plot(xys[:, 0], xys[:, 1], 'b')

  ps = np.array([
    [(1.0, 0,  1.0), (1.0, 3,  1.0), (1.0, 4,  0.0), (1.0, 6,  1.0)],
    [(1.5, 0,  1.0), (1.5, 3,  1.0), (1.5, 4,  1.0), (1.5, 6,  0.0)],
    [(2.0, 0, -1.0), (2.0, 3, -2.0), (2.0, 4, -1.0), (2.0, 6, -1.0)],
    [(3.0, 0,  0.0), (3.0, 3,  0.0), (3.0, 4,  1.0), (3.0, 7,  0.0)],
    [(4.0, 0,  1.0), (4.0, 3,  4.0), (4.0, 4,  1.0), (4.0, 6,  1.0)],
    [(5.0, 0,  0.0), (5.0, 3,  0.0), (5.0, 4,  0.0), (5.0, 6,  0.0)],
  ])
  #ps = [[np.array([x,y,z]) for x,y,z in rows] for rows in ps]
  us = np.linspace(0., 1., 50)
  vs = np.linspace(0., 1., 51)

  xyzs_interp = np.array([[cubic_clamped_bspline2(u, v, ps) for u in us] for v in vs])
  xs = xyzs_interp[:, :, 0][:]
  ys = xyzs_interp[:, :, 1][:]
  zs = xyzs_interp[:, :, 2][:]

  fig = plt.figure()#subplot(2, 1, 1)
  ax = fig.add_subplot(111, projection='3d')
  ax.scatter(xs, ys, zs)#, marker=m)

  xs = ps[:, :, 0][:]
  ys = ps[:, :, 1][:]
  zs = ps[:, :, 2][:]
  ax.scatter(xs, ys, zs)#, marker=m)
  ax.legend(['interpolated', 'control points'])


  plt.show()

if __name__=='__main__':
  main()
