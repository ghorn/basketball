#pragma once

#include <algorithm>              // for max, min
#include <eigen3/Eigen/Dense>     // for Matrix, DenseCoeffsBase, DenseBase<>::ConstantReturnType
#include <glm/glm.hpp>            // for vec3, dvec3, operator*, vec, vec<>::(anonymous), operator+
#include <memory>                 // for allocator_traits<>::value_type
#include <utility>                // for pair, make_pair
#include <vector>                 // for vector

#include "bb3d/assert.hpp"             // for ASSERT
#include "bb3d/shader/colorlines.hpp"  // for ColoredVec3, ColorLines
#include "bb3d/shader/cubemesh.hpp"    // for Cubemesh
#include "bb3d/shader/gridmesh.hpp"    // for Gridmesh
#include "bb3d/shader/lines.hpp"       // for Lines
#include "bspline.hpp"            // for Surface
#include "problem/backboard.hpp"  // for Backboard
#include "problem/hoop.hpp"       // for Hoop, Hoop::kRimDiameter
#include "problem/problem.hpp"    // for Problem
#include "problem/shot.hpp"       // for Bounce, Sample, Shot

template <typename T>
std::vector<std::vector<T> > SingletonVector(const std::vector<T> xs) {
  std::vector<std::vector<T> > ret;
  ret.push_back(xs);
  return ret;
}

class ProblemVisualization {
public:
  ProblemVisualization();
  ~ProblemVisualization() = default;
  void Draw(const glm::mat4 &view, const glm::mat4 &proj);

  template<int NU_OBJ, int NV_OBJ, int NU_VIS, int NV_VIS, int NX, int NY>
  void Update(const Eigen::Matrix<glm::dvec3, NX, NY> &control_points) {
    // shot and bounce lines
    const std::vector<Sample> samples =
      Problem<NX, NY>::template ComputeShots<NU_OBJ, NV_OBJ>(control_points);

    // shots
    // ------------------------------------------------------
    std::vector<std::vector<ColoredVec3> > shot_lines;
    std::vector<std::vector<ColoredVec3> > bounce_lines;
    // histogram range
    float min_x = (float)samples[0].bounce_.landing_point_.x;
    float max_x = (float)samples[0].bounce_.landing_point_.x;
    float min_y = (float)samples[0].bounce_.landing_point_.y;
    float max_y = (float)samples[0].bounce_.landing_point_.y;
    for (const Sample &sample : samples) {
      const Shot &shot = sample.shot_;
      const Bounce &bounce = sample.bounce_;

      min_x = std::min(min_x, (float)bounce.landing_point_.x);
      max_x = std::max(max_x, (float)bounce.landing_point_.x);
      min_y = std::min(min_y, (float)bounce.landing_point_.y);
      max_y = std::max(max_y, (float)bounce.landing_point_.y);

      // Color shot by how close it is to going in.
      double dist = bounce.XYDistanceFromHoop();
      float r = static_cast<float>(dist/Hoop::kRimDiameter);
      if (r < 0) {
        r = 0;
      }
      if (r > 1) {
        r = 1;
      }
      float g = 1 - r;
      glm::vec4 bounce_color = {r, g, 0, 0.6};
      glm::vec4 shot_color = {r, g, 0, 0.4};
      const std::vector<ColoredVec3> shot_arc = shot.DrawArc(shot_color);
      const std::vector<ColoredVec3> bounce_arc = bounce.DrawArc(bounce_color);
      shot_lines.push_back(shot_arc);
      bounce_lines.push_back(bounce_arc);
    }
    shot_lines_vis_.Update(shot_lines);
    bounce_lines_vis_.Update(bounce_lines);

    // histogram
    constexpr int nx_hist = 24;
    constexpr int ny_hist = 24;
    Eigen::Matrix<int, nx_hist, ny_hist> histogram = Eigen::Matrix<int, nx_hist, ny_hist>::Zero();
    int max_count = 0;
    for (const Sample &sample : samples) {
      const glm::vec3 &landing_point = sample.bounce_.landing_point_;
      int kx = static_cast<int>(0.5 + float(nx_hist - 1) * (landing_point.x - min_x) / (max_x - min_x));
      int ky = static_cast<int>(0.5 + float(ny_hist - 1) * (landing_point.y - min_y) / (max_y - min_y));
      ASSERT(kx >= 0);
      ASSERT(kx < nx_hist);
      ASSERT(ky >= 0);
      ASSERT(ky < ny_hist);
      histogram(kx, ky)++;
      max_count = std::max(max_count, histogram(kx, ky));
    }
    Eigen::Matrix<std::pair<float, glm::vec3>, nx_hist, ny_hist> histogram_float;
    const float max_z = -2.f;
    const float min_z = -1.f;
    const glm::vec3 warm = {0.5, 0.7, 0};
    const glm::vec3 cold = {0, 0.4, 1};
    for (int kx=0; kx<nx_hist; kx++) {
      for (int ky=0; ky<ny_hist; ky++) {
        const float z = (float)histogram(kx, ky) / (float)max_count;
        const glm::vec3 col = z * warm + (1 - z) * cold;
        histogram_float(kx, ky) = std::make_pair(min_z + z*(max_z - min_z), col);
      }
    }
    histogram_vis_.Update(histogram_float, min_x, max_x, min_y, max_y);

    // Rim
    rim_vis_.Update(SingletonVector(Hoop::DrawArc()));

    // backboard
    Surface<NU_VIS, NV_VIS> surface = Backboard<NX, NY>::template Interpolate<NU_VIS, NV_VIS>(control_points);
    backboard_vis_.Update(surface.position);

    // tangents
    std::vector<glm::vec3> tangents;
    std::vector<glm::vec3> normals;
    for (int ku=0; ku<NU_VIS; ku++) {
      for (int kv=0; kv<NV_VIS; kv++) {
        const glm::dvec3 &position = surface.position(ku, kv);
        const glm::dvec3 &tangent_u = surface.tangent_u(ku, kv);
        const glm::dvec3 &tangent_v = surface.tangent_v(ku, kv);
        tangents.push_back(position);
        tangents.push_back(position + 0.1*tangent_u);
        tangents.push_back(position);
        tangents.push_back(position + 0.1*tangent_v);

        if (ku > 0 && ku < NU_VIS - 1 && kv > 0 && kv < NV_VIS - 1) {
          normals.push_back(position);
          normals.push_back(position + surface.normal(ku, kv));
        }
      }
    }
    backboard_tangents_vis_.Update(SingletonVector(tangents));
    backboard_normals_vis_.Update(SingletonVector(normals));

    // control points
    std::vector<glm::vec3> control_point_vec;
    for (int kx=0; kx<NX; kx++) {
      for (int ky=0; ky<NY; ky++) {
        const glm::dvec3 point = control_points(kx, ky);
        control_point_vec.push_back(point);
      }
    }
    control_points_vis_.Update(SingletonVector(control_point_vec));
  }

  void HandleKeyPress(const int key);
private:
  static Eigen::Matrix<glm::vec3, 2, 2> CourtCorners();

  bool shots_on_ = false;
  bool bounces_on_ = true;
  bool normals_on_ = false;
  bool tangents_on_ = false;
  bool court_on_ = true;
  bool control_points_on_ = true;
  bool histogram_on_ = true;
  bool wireframe_on_ = false;

  Gridmesh backboard_vis_;
  Lines rim_vis_;
  Gridmesh court_vis_;
  Lines backboard_tangents_vis_;
  Lines backboard_normals_vis_;
  ColorLines shot_lines_vis_;
  ColorLines bounce_lines_vis_;
  Lines control_points_vis_;
  Cubemesh histogram_vis_;
};
