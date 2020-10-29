#pragma once

#include <string>

#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

struct CatShader {
  GLint uniModel;
  GLint uniView;
  GLint uniProj;

  GLuint textures[2];
  GLuint shaderProgram;
  GLuint fragmentShader;
  GLuint vertexShader;
  GLuint ebo;
  GLuint vbo;
  GLuint vao;
};

CatShader CreateCatShader(const std::string &image_path);
void DrawCatShader(const CatShader &cat_shader,
                   const glm::mat4 &model,
                   const glm::mat4 &view,
                   const glm::mat4 &proj);
void FreeCatGlResources(const CatShader &cat_shader);
