#pragma once

#include <cstdio>
#include <glm/glm.hpp>

#define ASSERT(expr) {                                                  \
  if (!(expr)) {                                                        \
    fprintf(stderr, "Assertion '" #expr "' failed at %s, line %d.\n", __FILE__, __LINE__); \
    exit(EXIT_FAILURE);                                                 \
  } \
}

const double g_accel = 9.81;

class Shot {
public:
  Shot(glm::dvec3 shot_point, glm::dvec3 bounce_point) {
    shot_point_ = shot_point;
    bounce_point_ = bounce_point;

    const double pz_shot = shot_point_.z;
    const double pz_bounce = bounce_point_.z;

    // assume this so that there's only one shot position
    vz_bounce_ = 0.5;
    ASSERT(pz_shot > pz_bounce);

    // v^2 == v0^2 + 2*a*(p - p0)
    vz_shot_ = - sqrt(vz_bounce_ * vz_bounce_ - 2 * g_accel * (pz_bounce - pz_shot));
    // v = v0 + a*t
    bounce_time_ = (vz_bounce_ - vz_shot_) / g_accel;
    ASSERT(bounce_time_ > 0);

    // px = px0 + vx*t
    vx_ = (bounce_point_.x - shot_point_.x) / bounce_time_;
    vy_ = (bounce_point_.y - shot_point_.y) / bounce_time_;
  }

  glm::dvec3 shot_point_;
  glm::dvec3 bounce_point_;
  double vz_bounce_;
  double vz_shot_;
  double vx_;
  double vy_;
  double bounce_time_;

  template <int N>
  void DrawArc(std::array<glm::vec3, N> *array) const {
    for (int k=0; k<N; k++) {
      const double t = k * bounce_time_ / (N - 1);
      (*array)[k].x = static_cast<float>(shot_point_.x + vx_ * t);
      (*array)[k].y = static_cast<float>(shot_point_.y + vy_ * t);
      (*array)[k].z = static_cast<float>(shot_point_.z + vz_shot_ * t + 0.5*g_accel*t*t);
    }
  }

//private:
//  glm::vec3d ComputeShotTime() {
//
//  }
  
};
