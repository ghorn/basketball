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
  int current_num_indices;
};

GridmeshShader CreateGridmesh();
void DrawGridmesh(const GridmeshShader &gridmesh,
                   const glm::mat4 &view,
                   const glm::mat4 &proj);
void FreeGridmeshGlResources(const GridmeshShader &gridmesh);

void UpdateGridmesh(GridmeshShader &gridmesh, float *vertices, int rows, int cols);

template <int NU, int NV>
void UpdateGridmeshFromMatrix(GridmeshShader &gridmesh, Eigen::Matrix<glm::dvec3, NU, NV> mat) {
  std::vector<float> vec;
  for (int ku=0; ku<NU; ku++) {
    for (int kv=0; kv<NV; kv++) {
      vec.push_back(static_cast<float>(mat(ku, kv).x));
      vec.push_back(static_cast<float>(mat(ku, kv).y));
      vec.push_back(static_cast<float>(mat(ku, kv).z));
    }
  }
  UpdateGridmesh(gridmesh, vec.data(), (int)NU, (int)NV);
}

template <int NU, int NV>
void UpdateGridmeshFromMatrix2(GridmeshShader &gridmesh, Eigen::Matrix<glm::dvec3, NU, NV> mat) {
  std::vector<float> vec;
  for (int ku=0; ku<NU; ku++) {
    for (int kv=0; kv<NV; kv++) {
      vec.push_back(static_cast<float>(mat(ku, kv).x-2));
      vec.push_back(static_cast<float>(mat(ku, kv).y+1));
      vec.push_back(static_cast<float>(mat(ku, kv).z));
    }
  }
  UpdateGridmesh(gridmesh, vec.data(), (int)NU, (int)NV);
}

template <int NU, int NV>
void UpdateGridmeshFromMatrix3(GridmeshShader &gridmesh, Eigen::Matrix<glm::dvec3, NU, NV> mat) {
  std::vector<float> vec;
  int nu_ = 0;
  int nv_ = 0;
  for (int ku=2; ku<NU-4; ku++) {
    nu_+=1;
    nv_ = 0;
    for (int kv=5; kv<NV-3; kv++) {
      nv_ += 1;
      vec.push_back(static_cast<float>(mat(ku, kv).x-2));
      vec.push_back(static_cast<float>(mat(ku, kv).y+1));
      vec.push_back(static_cast<float>(mat(ku, kv).z));
    }
  }
  UpdateGridmesh(gridmesh, vec.data(), nu_, nv_);
}
