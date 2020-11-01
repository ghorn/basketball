#include "lines.hpp"

#include <array>
#include <iostream>

#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

#include "assert.hpp"
#include "shader/compile.hpp"

// Shader sources
const GLchar* lineVertexShaderSource = R"glsl(
  #version 400 core
  layout (location = 0) in vec3 position;
  uniform mat4 view;
  uniform mat4 proj;
  void main()
  {
    gl_Position = proj * view * vec4(position, 1.0);
    gl_PointSize = 3;
  }
)glsl";

const GLchar* lineFragmentShaderSource = R"glsl(
  #version 400 core
  out vec4 output_color;
  uniform vec4 color;
  void main()
  {
    output_color = color;
  }
)glsl";


LineShader CreateLineShader() {
  LineShader line_shader;
  line_shader.shaderProgram =
    CompileAndLinkVertexFragmentShaderProgram(lineVertexShaderSource, lineFragmentShaderSource);

  line_shader.current_buffer_size = 0;

  // set up vertex data (and buffer(s)) and configure vertex attributes
  // ------------------------------------------------------------------
  glGenVertexArrays(1, &line_shader.VAO);
  glGenBuffers(1, &line_shader.VBO);

  // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
  glBindVertexArray(line_shader.VAO);
  glBindBuffer(GL_ARRAY_BUFFER, line_shader.VBO);
  glBufferData(GL_ARRAY_BUFFER,
               line_shader.current_buffer_size,
               nullptr,
               GL_DYNAMIC_DRAW);

  GLint posAttrib = 0; // layout = 0 above
  glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
  glEnableVertexAttribArray(posAttrib);

  // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
  // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
  glBindVertexArray(0);

  return line_shader;
}

void DrawLines(LineShader &line_shader,
               const glm::mat4 &view,
               const glm::mat4 &proj,
               const glm::vec4 &color,
               const GLenum mode) {
  // draw triangle
  glUseProgram(line_shader.shaderProgram);
  glBindVertexArray(line_shader.VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized

  // Set up transformations
  GLint uniView = glGetUniformLocation(line_shader.shaderProgram, "view");
  GLint uniProj = glGetUniformLocation(line_shader.shaderProgram, "proj");
  glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));
  glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));

  GLint uniColor = glGetUniformLocation(line_shader.shaderProgram, "color");
  glUniform4f(uniColor, color.r, color.g, color.b, color.a);

  GLint offset = 0;
  for (const GLint segment_size : line_shader.segment_sizes) {
    glDrawArrays(mode, offset, segment_size);
    offset += segment_size;
  }
}


void UpdateLines(LineShader &line_shader,
                 const std::vector<std::vector<glm::vec3> > &segments) {
  // Massage the data.
  // TODO(greg): static assert that std::vector<glm::vec3> is packed and just reinterpret cast
  std::vector<float> buffer_data;
  line_shader.segment_sizes.resize(0);
  for (const std::vector<glm::vec3> &segment : segments) {
    const GLint segment_size = static_cast<GLint>(segment.size());
    line_shader.segment_sizes.push_back(segment_size);
    for (const glm::vec3 &vertex : segment) {
      buffer_data.push_back(vertex.x);
      buffer_data.push_back(vertex.y);
      buffer_data.push_back(vertex.z);
    }
  }

  // bind the buffer
  //glBindVertexArray(line_shader.VAO);
  glBindBuffer(GL_ARRAY_BUFFER, line_shader.VBO);

  const GLint buffer_size = static_cast<GLint>(sizeof(float) * buffer_data.size());
  if (buffer_size == line_shader.current_buffer_size) {
    // if the size of data is the same, just update the buffer
    glBufferSubData(GL_ARRAY_BUFFER, 0, buffer_size, buffer_data.data());
  } else {
    // if the size of data has changed, we have to reallocate GPU memory
    glBufferData(GL_ARRAY_BUFFER, buffer_size, buffer_data.data(), GL_DYNAMIC_DRAW);
    line_shader.current_buffer_size = buffer_size;
  }

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  //glBindVertexArray(0);
}
