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

  template<int NU_OBJ, int NV_OBJ, int NU_VIS, int NV_VIS, int NX, int NY>
  void Update(const Eigen::Matrix<glm::dvec3, NX, NY> &control_points) {
    // shot and bounce lines
    const std::vector<Sample> samples =
      Problem<NX, NY>::template ComputeShots<NU_OBJ, NV_OBJ>(control_points);

    // shots
    std::vector<std::vector<ColoredVec3> > shot_lines;
    std::vector<std::vector<ColoredVec3> > bounce_lines;
    for (const Sample &sample : samples) {
      const Shot &shot = sample.shot_;
      const Bounce &bounce = sample.bounce_;

      glm::vec4 good_shot_color = {0.6, 0.6, 0.6, 1.0};
      glm::vec4 bad_shot_color = {0.6, 0.6, 0.6, 0.5};
      glm::vec4 good_bounce_color = {0.1, 0.7, 0.2, 0.6};
      glm::vec4 bad_bounce_color =  {0.8, 0.1, 0.2, 0.6};
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

  Gridmesh backboard_vis_;
  Lines rim_vis_;
  Gridmesh court_vis_;
  Lines backboard_tangents_vis_;
  Lines backboard_normals_vis_;
  ColorLines shot_lines_vis_;
  ColorLines bounce_lines_vis_;
  Lines control_points_vis_;
};
