#pragma once

#include <vector>

#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "shader/shader.hpp"

class Lines {
public:
  Lines();
  ~Lines() = default;
  void Update(const std::vector<std::vector<glm::vec3> > &vertices);
  void Draw(const glm::mat4 &view, const glm::mat4 &proj, const glm::vec4 &color, const GLenum mode);

  float point_size_ = 1;

private:
  Shader shader_;
  GLuint vao_;
  GLuint vbo_;
  std::vector<GLint> segment_sizes_;
  GLint current_buffer_size_;
};
