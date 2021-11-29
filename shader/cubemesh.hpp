#pragma once

#include <GL/glew.h>           // for GLuint, GLint
#include <eigen3/Eigen/Dense>  // for Matrix, Dynamic, DenseCoeffsBase
#include <glm/glm.hpp>         // for vec3, mat4
#include <utility>             // for pair
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "shader/shader.hpp"   // for Shader

struct Cubemesh {
  Cubemesh();
  ~Cubemesh();

  void Draw(const glm::mat4 &view, const glm::mat4 &proj);
  void Update(const Eigen::Matrix<std::pair<float, glm::vec3>, Eigen::Dynamic, Eigen::Dynamic> &grid,
              const float min_x, const float max_x,
              const float min_y, const float max_y);
  template <int NU, int NV>
  void Update(const Eigen::Matrix<std::pair<float, glm::vec3>, NU, NV> &mat,
              const float min_x, const float max_x,
              const float min_y, const float max_y) {
    Eigen::Matrix<std::pair<float, glm::vec3>, Eigen::Dynamic, Eigen::Dynamic> dynamic_mat(NU, NV);
    for (int ku=0; ku<NU; ku++) {
      for (int kv=0; kv<NV; kv++) {
        dynamic_mat(ku, kv) = mat(ku, kv);
      }
    }
    Update(dynamic_mat, min_x, max_x, min_y, max_y);
  }

private:
  Shader shader_;
  GLuint vao_;
  GLuint vbo_;
  GLuint ebo_;
  int num_indices_;
  GLint vertex_buffer_size_ = 0;
  GLint index_buffer_size_ = 0;
};
