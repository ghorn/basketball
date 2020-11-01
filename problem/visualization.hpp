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

  template<int NX, int NY>
  void Update(const Problem<NX, NY> &problem) {
    // shot lines
    const glm::dvec3 shot_point(4, 0.5, 1);
    const std::vector<Shot> shots = problem.template ComputeShots<10, 15>(shot_point);

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

    // backboard
    Surface<20, 30> surface = problem.backboard_.template Interpolate<20, 30>();
    UpdateGridmeshFromMatrix(backboard_vis_, surface.position);

    // tangents
    std::vector<glm::vec3> tangents;
    for (int ku=0; ku<20; ku++) {
      for (int kv=0; kv<30; kv++) {
        const glm::dvec3 &position = surface.position(ku, kv);
        const glm::dvec3 &tangent_u = surface.tangent_u(ku, kv);
        const glm::dvec3 &tangent_v = surface.tangent_v(ku, kv);
        tangents.push_back(position);
        tangents.push_back(position + 0.1*tangent_u);
        tangents.push_back(position);

        tangents.push_back(position + 0.1*tangent_v);
      }
    }
    UpdateLines(backboard_tangents_vis_, SingletonVector(tangents));

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
  LineShader shot_lines_vis_;
  LineShader control_points_vis_;
};
