#!/usr/bin/env python3

import argparse
import numpy as np
import time
import matplotlib.pyplot as plt

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

def cubic_clamped_bspline(s, ps):
  n_extra = 2
  return cubic_bspline(s, n_extra*[ps[0]] + ps + n_extra*[ps[-1]])

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

  plt.plot([p[0] for p in ps], [p[1] for p in ps], 'r-.o')
  plt.plot(xys[:, 0], xys[:, 1], 'b')
  plt.show()

if __name__=='__main__':
  main()
