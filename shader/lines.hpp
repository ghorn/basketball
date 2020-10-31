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
  GLint num_vertices;
};

LineShader CreateLineShader(float *vertices, int num_vertices);
void DrawLines(LineShader &line_shader,
               const glm::mat4 &view,
               const glm::mat4 &proj,
               const glm::vec4 &color,
               const GLenum mode);
void UpdateLines(LineShader &line_shader, float *new_verts, int num_vertices);
