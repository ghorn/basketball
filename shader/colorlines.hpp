#pragma once

#include <vector>

#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

struct ColorLineShader {
  GLint shaderProgram;
  GLuint VAO;
  GLuint VBO;
  GLint current_buffer_size;
  std::vector<GLint> segment_sizes;
};

struct ColoredVec3 {
  glm::vec3 position;
  glm::vec4 color;
};

ColorLineShader CreateColorLineShader();
void DrawColorLines(ColorLineShader &line_shader,
                    const glm::mat4 &view,
                    const glm::mat4 &proj,
                    const GLenum mode);
void UpdateColorLines(ColorLineShader &line_shader,
                      const std::vector<std::vector<ColoredVec3> > &vertices);
