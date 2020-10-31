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
  GLint current_num_vertices;
};

LineShader CreateLineShader(const std::vector<glm::vec3> &vertices);
void DrawLines(LineShader &line_shader,
               const glm::mat4 &view,
               const glm::mat4 &proj,
               const glm::vec4 &color,
               const GLenum mode);
void UpdateLines(LineShader &line_shader, const std::vector<glm::vec3> &vertices);
