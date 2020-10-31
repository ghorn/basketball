#include "lines.hpp"

#include <array>
#include <iostream>

#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

#include "assert.hpp"

// Shader sources
const GLchar* lineVertexShaderSource = R"glsl(
  #version 400 core
  in vec3 position;
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

GLuint CreateLineShaderProgram() {
  // build and compile our shader program
  // ------------------------------------
  // vertex shader
  GLint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &lineVertexShaderSource, NULL);
  glCompileShader(vertexShader);
  // check for shader compile errors
  GLint success;
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if (!success)
  {
    char infoLog[512];
    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    exit(EXIT_FAILURE);
  }
  // fragment shader
  GLint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &lineFragmentShaderSource, NULL);
  glCompileShader(fragmentShader);
  // check for shader compile errors
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  if (!success)
  {
    char infoLog[512];
    glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
    std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    exit(EXIT_FAILURE);
  }
  // link shaders
  GLint shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);
  // check for linking errors
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
  if (!success) {
    char infoLog[512];
    glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
    std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    exit(EXIT_FAILURE);
  }
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  return shaderProgram;
}


LineShader CreateLineShader(const std::vector<glm::vec3> &vertices) {
  const GLint num_vertices = static_cast<GLint>(vertices.size());
  std::vector<float> buffer_data;
  buffer_data.reserve(num_vertices);
  for (const glm::vec3 &vertex : vertices) {
      buffer_data.push_back(vertex.x);
      buffer_data.push_back(vertex.y);
      buffer_data.push_back(vertex.z);
  }

  LineShader line_shader;
  line_shader.shaderProgram = CreateLineShaderProgram();

  line_shader.current_num_vertices = num_vertices;

  // set up vertex data (and buffer(s)) and configure vertex attributes
  // ------------------------------------------------------------------
  glGenVertexArrays(1, &line_shader.VAO);
  glGenBuffers(1, &line_shader.VBO);

  // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
  glBindVertexArray(line_shader.VAO);
  glBindBuffer(GL_ARRAY_BUFFER, line_shader.VBO);
  glBufferData(GL_ARRAY_BUFFER,
               3*sizeof(float)*line_shader.current_num_vertices,
               buffer_data.data(),
               GL_DYNAMIC_DRAW);

  GLint posAttrib = glGetAttribLocation(line_shader.shaderProgram, "position");
  glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
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

  glDrawArrays(mode, 0, line_shader.current_num_vertices);
}


void UpdateLines(LineShader &line_shader, const std::vector<glm::vec3> &vertices) {
  // Massage the data.
  // TODO(greg): static assert that std::vector<glm::vec3> is packed and just reinterpret cast
  const GLint new_num_vertices = static_cast<GLint>(vertices.size());
  std::vector<float> buffer_data;
  buffer_data.reserve(vertices.size());
  for (const glm::vec3 &vertex : vertices) {
      buffer_data.push_back(vertex.x);
      buffer_data.push_back(vertex.y);
      buffer_data.push_back(vertex.z);
  }

  // bind the buffer
  //glBindVertexArray(line_shader.VAO);
  glBindBuffer(GL_ARRAY_BUFFER, line_shader.VBO);

  const GLint buffer_size = static_cast<GLint>(3 * sizeof(float) * new_num_vertices);
  if (new_num_vertices == line_shader.current_num_vertices) {
    // if the size of data is the same, just update the buffer
    glBufferSubData(GL_ARRAY_BUFFER, 0, buffer_size, buffer_data.data());
  } else {
    fprintf(stderr, "Can't change number of vertices!\n");
  }
}
