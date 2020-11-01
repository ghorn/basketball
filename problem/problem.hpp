#pragma once

#include <tuple>
#include <vector>

#include <eigen3/Eigen/Dense>
#include <glm/glm.hpp>

#include "problem/backboard.hpp"
#include "problem/shot.hpp"

template <int NX, int NY>
class Problem {
public:
  Backboard<NX, NY> backboard_;

  template <int NU, int NV>
  std::vector<std::pair<Shot, Bounce> > ComputeShots() const {
    Surface surface = backboard_.template Interpolate<NU, NV>();
    const Eigen::Matrix<glm::dvec3, NU, NV> &bounce_points = surface.position;

    const glm::dvec3 shot_point(0.5, 4, 0);

    std::vector<std::pair<Shot, Bounce> > result;
    for (int ku=1; ku<NU-1; ku++) {
      for (int kv=1; kv<NV-1; kv++) {
        Shot shot(shot_point, bounce_points(ku, kv));
        Bounce bounce(shot.bounce_point_,
                      shot.BounceVel(),
                      surface.normal(ku, kv));;
        result.push_back(std::make_pair(shot, bounce));
      }
    }
    return result;
  }
private:
};
