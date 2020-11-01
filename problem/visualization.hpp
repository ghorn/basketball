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
  void FreeResources();
  void Draw(const glm::mat4 &view, const glm::mat4 &proj);

  template<int NU, int NV, int NX, int NY>
  void Update(const Problem<NX, NY> &problem) {
    // shot and bounce lines
    std::vector<Shot> shots;
    std::vector<Bounce> bounces;
    problem.template ComputeShots<10, 15>(&shots, &bounces);

    // shots
    std::vector<std::vector<glm::vec3> > shot_lines;
    for (const Shot &shot : shots) {
      std::array<glm::vec3, 256> shot_arc;
      shot.DrawArc<256>(&shot_arc);

      std::vector<glm::vec3> segment;
      for (glm::vec3 v3 : shot_arc) {
        segment.push_back(v3);
      }
      shot_lines.push_back(segment);
    }
    UpdateLines(shot_lines_vis_, shot_lines);

    // bounces
    std::vector<std::vector<glm::vec3> > bounce_lines;
    for (const Bounce &bounce : bounces) {
      std::array<glm::vec3, 256> bounce_arc;
      bounce.DrawArc<256>(&bounce_arc);

      std::vector<glm::vec3> segment;
      for (glm::vec3 v3 : bounce_arc) {
        segment.push_back(v3);
      }
      bounce_lines.push_back(segment);
    }
    UpdateLines(bounce_lines_vis_, bounce_lines);

    // backboard
    Surface<NU, NV> surface = problem.backboard_.template Interpolate<NU, NV>();
    UpdateGridmeshFromMatrix(backboard_vis_, surface.position);

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
    UpdateLines(backboard_tangents_vis_, SingletonVector(tangents));
    UpdateLines(backboard_normals_vis_, SingletonVector(normals));

    // control points
    std::vector<glm::vec3> control_points;
    for (int kx=0; kx<problem.backboard_.control_points_.rows(); kx++) {
      for (int ky=0; ky<problem.backboard_.control_points_.cols(); ky++) {
        const glm::dvec3 point = problem.backboard_.control_points_(kx, ky);
        control_points.push_back(point);
      }
    }
    UpdateLines(control_points_vis_, SingletonVector(control_points));
  }

private:
  GridmeshShader backboard_vis_;
  LineShader backboard_tangents_vis_;
  LineShader backboard_normals_vis_;
  LineShader shot_lines_vis_;
  LineShader bounce_lines_vis_;
  LineShader control_points_vis_;
};
