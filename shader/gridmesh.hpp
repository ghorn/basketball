#pragma once

#pragma once

#include <eigen3/Eigen/Dense>

#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

struct GridmeshShader {
  GLint shaderProgram;
  GLuint VAO;
  GLuint VBO;
  GLuint EBO;
  GLuint texture;
  int num_indices;
  GLint vertex_buffer_size = 0;
  GLint index_buffer_size = 0;
};

GridmeshShader CreateGridmesh(const std::string &image_path);
void DrawGridmesh(const GridmeshShader &gridmesh,
                   const glm::mat4 &view,
                   const glm::mat4 &proj);
void FreeGridmeshGlResources(const GridmeshShader &gridmesh);

void UpdateGridmesh(GridmeshShader &gridmesh,
                    const Eigen::Matrix<glm::vec3, Eigen::Dynamic, Eigen::Dynamic> &grid);

template <int NU, int NV>
void UpdateGridmeshFromMatrix(GridmeshShader &gridmesh,
                              const Eigen::Matrix<glm::dvec3, NU, NV> &mat) {
  Eigen::Matrix<glm::vec3, Eigen::Dynamic, Eigen::Dynamic> dynamic_mat(NU, NV);
  for (int ku=0; ku<NU; ku++) {
    for (int kv=0; kv<NV; kv++) {
      dynamic_mat(ku, kv) = mat(ku, kv);
    }
  }
  UpdateGridmesh(gridmesh, dynamic_mat);
}
