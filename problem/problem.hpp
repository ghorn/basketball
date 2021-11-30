#pragma once

#include <algorithm>           // for max
#include <eigen3/Eigen/Dense>  // for Matrix
#include <glm/glm.hpp>         // for dvec3
#include <vector>              // for vector

#include "bb3d/assert.hpp"        // for ASSERT
#include "problem/backboard.hpp"  // for Backboard
#include "problem/shot.hpp"       // for Sample, Bounce

template <int NX, int NY>
class Problem {
 public:
  template <int NU, int NV>
  static std::vector<Sample> ComputeShots(const Eigen::Matrix<glm::dvec3, NX, NY> &control_points) {
    Surface surface = Backboard<NX, NY>::template Interpolate<NU, NV>(control_points);
    const Eigen::Matrix<glm::dvec3, NU, NV> &bounce_points = surface.position;

    std::vector<Sample> result;

    const int num_sp_x = 5;
    const int num_sp_y = 4;
    for (int k_sp_x = 0; k_sp_x < num_sp_x; k_sp_x++) {
      const double sp_x = k_sp_x / static_cast<double>(num_sp_x - 1);
      for (int k_sp_y = 0; k_sp_y < num_sp_y; k_sp_y++) {
        const double sp_y = k_sp_y / static_cast<double>(num_sp_y - 1);

        const glm::dvec3 shot_point(1.5 * (2 * sp_x + -1), 3 + sp_y * 2, 0);

        ASSERT(NU > 2);
        ASSERT(NV > 2);
        for (int ku = 1; ku < NU - 1; ku++) {
          for (int kv = 1; kv < NV - 1; kv++) {
            result.push_back(Sample(shot_point, bounce_points(ku, kv), surface.normal(ku, kv)));
          }
        }
      }
    }
    return result;
  }

  template <int NU, int NV>
  static double ObjectiveFunction(const Eigen::Matrix<glm::dvec3, NX, NY> &control_points) {
    std::vector<Sample> samples = ComputeShots<NU, NV>(control_points);
    double objective = 0;
    for (const Sample &sample : samples) {
      ASSERT(!sample.bounce_.lower_than_hoop_);
      double xydist = sample.bounce_.XYDistanceFromHoop();
      objective += xydist * xydist;
    }
    return objective;
  }

 private:
};
