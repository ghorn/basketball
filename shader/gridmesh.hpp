#pragma once

#include <eigen3/Eigen/Dense>

#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "shader/shader.hpp"

struct Gridmesh {
  Gridmesh(const std::string &image_path);
  ~Gridmesh();

  void Update(const Eigen::Matrix<glm::vec3, Eigen::Dynamic, Eigen::Dynamic> &grid);
  void Draw(const glm::mat4 &view, const glm::mat4 &proj);
  template <int NU, int NV>
  void Update(const Eigen::Matrix<glm::dvec3, NU, NV> &mat) {
    Eigen::Matrix<glm::vec3, Eigen::Dynamic, Eigen::Dynamic> dynamic_mat(NU, NV);
    for (int ku=0; ku<NU; ku++) {
      for (int kv=0; kv<NV; kv++) {
        dynamic_mat(ku, kv) = mat(ku, kv);
      }
    }
    Update(dynamic_mat);
  }

private:
  Shader shader_;
  GLuint vao_;
  GLuint vbo_;
  GLuint ebo_;
  GLuint texture_;
  int num_indices_;
  GLint vertex_buffer_size_ = 0;
  GLint index_buffer_size_ = 0;
};
