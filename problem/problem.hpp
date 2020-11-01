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
  void ComputeShots(const glm::dvec3 &shot_point,
                    std::vector<Shot> *shots,
                    std::vector<Bounce> *bounces) const {
    Surface surface = backboard_.template Interpolate<NU, NV>();
    const Eigen::Matrix<glm::dvec3, NU, NV> &bounce_points = surface.position;

    shots->clear();
    bounces->clear();
    for (int ku=1; ku<NU-1; ku++) {
      for (int kv=1; kv<NV-1; kv++) {
        Shot shot(shot_point, bounce_points(ku, kv));
        shots->push_back(shot);
        Bounce bounce(shot.bounce_point_,
                      shot.BounceVel(),
                      surface.normal(ku, kv));;
        bounces->push_back(bounce);
      }
    }
  }
private:
};
