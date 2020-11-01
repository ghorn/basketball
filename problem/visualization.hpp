#pragma once

#include "problem.hpp"

#include <eigen3/Eigen/Dense>
#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "problem/problem.hpp"
#include "shader/gridmesh.hpp"
#include "shader/lines.hpp"

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

  template<int NU, int NV, int NX, int NY>
  void Update(const Problem<NX, NY> &problem) {
    // shot and bounce lines
    const std::vector<std::pair<Shot, Bounce> > shots_n_bounces =
      problem.template ComputeShots<10, 15>();

    // shots
    std::vector<std::vector<ColoredVec3> > shot_lines;
    std::vector<std::vector<ColoredVec3> > bounce_lines;
    for (const std::pair<Shot, Bounce> &shot_n_bounce : shots_n_bounces) {
      const Shot &shot = shot_n_bounce.first;
      const Bounce &bounce = shot_n_bounce.second;

      glm::vec4 good_shot_color = {0.6, 0.6, 0.6, 1.0};
      glm::vec4 bad_shot_color = {0.6, 0.6, 0.6, 0.5};
      glm::vec4 good_bounce_color = {0.1, 0.7, 0.2, 1.0};
      glm::vec4 bad_bounce_color =  {0.8, 0.1, 0.2, 0.5};
      glm::vec4 shot_color = bounce.lower_than_hoop_ ? bad_shot_color : good_shot_color;
      glm::vec4 bounce_color = bounce.lower_than_hoop_ ? bad_bounce_color : good_bounce_color;

      const std::vector<ColoredVec3> shot_arc = shot.DrawArc(shot_color);
      const std::vector<ColoredVec3> bounce_arc = bounce.DrawArc(bounce_color);
      shot_lines.push_back(shot_arc);
      bounce_lines.push_back(bounce_arc);
    }
    shot_lines_vis_.Update(shot_lines);
    bounce_lines_vis_.Update(bounce_lines);

    // Rim
    rim_vis_.Update(SingletonVector(Hoop::DrawArc()));

    // backboard
    Surface<NU, NV> surface = problem.backboard_.template Interpolate<NU, NV>();
    backboard_vis_.Update(surface.position);

    // tangents
    std::vector<glm::vec3> tangents;
    std::vector<glm::vec3> normals;
    for (int ku=0; ku<NU; ku++) {
      for (int kv=0; kv<NV; kv++) {
        const glm::dvec3 &position = surface.position(ku, kv);
        const glm::dvec3 &tangent_u = surface.tangent_u(ku, kv);
        const glm::dvec3 &tangent_v = surface.tangent_v(ku, kv);
        tangents.push_back(position);
        tangents.push_back(position + 0.1*tangent_u);
        tangents.push_back(position);
        tangents.push_back(position + 0.1*tangent_v);

        if (ku > 0 && ku < NU - 1 && kv > 0 && kv < NV - 1) {
          normals.push_back(position);
          normals.push_back(position + surface.normal(ku, kv));
        }
      }
    }
    backboard_tangents_vis_.Update(SingletonVector(tangents));
    backboard_normals_vis_.Update(SingletonVector(normals));

    // control points
    std::vector<glm::vec3> control_points;
    for (int kx=0; kx<problem.backboard_.control_points_.rows(); kx++) {
      for (int ky=0; ky<problem.backboard_.control_points_.cols(); ky++) {
        const glm::dvec3 point = problem.backboard_.control_points_(kx, ky);
        control_points.push_back(point);
      }
    }
    control_points_vis_.Update(SingletonVector(control_points));
  }

  void HandleKeyPress(const int key);
private:
  static Eigen::Matrix<glm::vec3, 2, 2> CourtCorners();

  bool shots_on_ = true;
  bool bounces_on_ = true;
  bool normals_on_ = true;
  bool tangents_on_ = true;
  bool court_on_ = true;
  bool control_points_on_ = true;

  Gridmesh backboard_vis_;
  Lines rim_vis_;
  Gridmesh court_vis_;
  Lines backboard_tangents_vis_;
  Lines backboard_normals_vis_;
  ColorLines shot_lines_vis_;
  ColorLines bounce_lines_vis_;
  Lines control_points_vis_;
};
