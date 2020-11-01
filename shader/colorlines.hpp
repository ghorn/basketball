#pragma once

#include <vector>

#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "shader/shader.hpp"

struct ColoredVec3 {
  glm::vec3 position;
  glm::vec4 color;
};

struct ColorLines {
public:
  ColorLines();
  ~ColorLines() = default;
  void Update(const std::vector<std::vector<ColoredVec3> > &vertices);
  void Draw(const glm::mat4 &view, const glm::mat4 &proj, const GLenum mode);

  float point_size_;

private:
  Shader shader_;
  GLuint vao_;
  GLuint vbo_;
  GLint current_buffer_size_;
  std::vector<GLint> segment_sizes_;
};

