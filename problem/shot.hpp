#pragma once

#include <glm/glm.hpp>            // for dvec3, vec<>::(anonymous), vec3, vec4, operator-, reflect
#include <cmath>                  // for sqrt, fabs
#include <vector>                 // for vector
#include <algorithm>              // for max

#include "assert.hpp"             // for ASSERT
#include "problem/hoop.hpp"       // for Hoop, Hoop::kRimHeight
#include "shader/colorlines.hpp"  // for ColoredVec3

const double g_accel = 9.81;

class Shot {
public:
  Shot(glm::dvec3 shot_point,
       glm::dvec3 bounce_point) {
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

  std::vector<ColoredVec3> DrawArc(const glm::vec4 &color) const {
    constexpr int N = 128;
    std::vector<ColoredVec3> ret;
    ret.reserve(N);
    for (int k=0; k<N; k++) {
      const double t = k * bounce_time_ / (N - 1);
      ColoredVec3 v;
      v.position.x = static_cast<float>(shot_point_.x + vx_ * t);
      v.position.y = static_cast<float>(shot_point_.y + vy_ * t);
      v.position.z = static_cast<float>(shot_point_.z + vz_shot_ * t + 0.5*g_accel*t*t);
      v.color = color;
      ret.push_back(v);
    }
    return ret;
  }

  glm::dvec3 BounceVel() const {
    return glm::dvec3(vx_, vy_, vz_bounce_);
  }
};


class Bounce {
public:
  Bounce(const glm::dvec3 &bounce_point,
         const glm::dvec3 &incoming_velocity,
         const glm::dvec3 &bounce_normal) : bounce_point_(bounce_point) {

    //glm::dvec3 normal = glm::cross(bounce_tangent_v, bounce_tangent_u);
    //ASSERT(glm::length(normal) > 1e-9);
    //normal = glm::normalize(normal);

    // I - 2.0 * dot(N, I) * N
    outgoing_velocity_ = glm::reflect(incoming_velocity, bounce_normal);

    const double vz0 = outgoing_velocity_.z;
    double pz0 = Hoop::kRimHeight + bounce_point_.z;
    lower_than_hoop_ = false;
    if (pz0 >= 0) {
      lower_than_hoop_ = true;
      pz0 = bounce_point_.z;
    }
    ASSERT(pz0 < 0);
    land_time_ = (-vz0 + sqrt(vz0*vz0 - 2 * pz0 * g_accel))/ g_accel;


    // landing point
    const double &t = land_time_;
    landing_point_.x = bounce_point_.x + outgoing_velocity_.x * t;
    landing_point_.y = bounce_point_.y + outgoing_velocity_.y * t;
    landing_point_.z = bounce_point_.z + outgoing_velocity_.z * t + 0.5*g_accel*t*t;
  }
  bool lower_than_hoop_;
  glm::dvec3 bounce_point_;
  glm::dvec3 outgoing_velocity_;
  double land_time_;
  glm::dvec3 landing_point_;

  double XYDistanceFromHoop() const {
    glm::dvec3 rim_center = Hoop::RimCenter();
    glm::dvec3 delta = rim_center - landing_point_;

    ASSERT(fabs(delta.z) < 1e-9);

    return sqrt(delta.x*delta.x + delta.y*delta.y);
  }

  std::vector<ColoredVec3> DrawArc(const glm::vec4 &color) const {
    constexpr int N = 128;
    std::vector<ColoredVec3> ret;
    ret.reserve(N);
    for (int k=0; k<N; k++) {
      const double t = k * land_time_ / (N - 1);
      ColoredVec3 v;
      v.position.x = static_cast<float>(bounce_point_.x + outgoing_velocity_.x * t);
      v.position.y = static_cast<float>(bounce_point_.y + outgoing_velocity_.y * t);
      v.position.z = static_cast<float>(bounce_point_.z + outgoing_velocity_.z * t + 0.5*g_accel*t*t);
      v.color = color;
      ret.push_back(v);
    }
    return ret;
  }
};

class Sample {
public:
  Sample(glm::dvec3 shot_point, glm::dvec3 bounce_point, glm::dvec3 normal) :
    shot_(shot_point, bounce_point),
    bounce_(shot_.bounce_point_, shot_.BounceVel(), normal)
  {
  }
  Shot shot_;
  Bounce bounce_;
  double objective;
};
