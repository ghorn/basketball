#pragma once

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
  std::vector<Shot> ComputeShots(const glm::dvec3 &shot_point) {
    Eigen::Matrix<glm::dvec3, NU, NV> bounce_points = backboard_.template Interpolate<NU, NV>();

    std::vector<Shot> shots;
    for (int ku=0; ku<NU; ku++) {
      for (int kv=0; kv<NV; kv++) {
        shots.push_back(Shot(shot_point, bounce_points(ku, kv)));
      }
    }

    return shots;
  }
  //Shot shots_[NU][NV];

  //SetShots() {
  //  Eigen::Matrix<glm::dvec3, NU, NV> bounce_points = backboard_.Interpolate();
  //}

//  template <int NU, int NV>
//  Eigen::Matrix<Shot, NU, NV> MakeShots;
private:
  
  //  template <int NU, int NV>
};
