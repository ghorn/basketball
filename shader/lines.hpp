#pragma once

//#include <array>

#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

//template <int N>
struct LineShader {
  GLint shaderProgram;
  GLuint VAO;
  GLuint VBO;
  GLint num_vertices;
};

LineShader CreateLineShader(float *vertices, int num_vertices);
void DrawLines(LineShader &line_shader,
               const glm::mat4 &view,
               const glm::mat4 &proj,
               const glm::vec4 &color,
               const GLenum mode);
