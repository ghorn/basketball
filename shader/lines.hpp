#pragma once

#include <vector>

#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

struct LineShader {
  GLint shaderProgram;
  GLuint VAO;
  GLuint VBO;
  GLint current_buffer_size;
  std::vector<GLint> segment_sizes;
  float point_size;
};

LineShader CreateLineShader();
void DrawLines(LineShader &line_shader,
               const glm::mat4 &view,
               const glm::mat4 &proj,
               const glm::vec4 &color,
               const GLenum mode);
void UpdateLines(LineShader &line_shader,
                 const std::vector<std::vector<glm::vec3> > &vertices);
