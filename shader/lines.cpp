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


static void CreateLineShaderBuffers(GLint shaderProgram,
                                    GLuint *VBO, GLuint *VAO,
                                    float *vertices, int num_vec3s) {
  // set up vertex data (and buffer(s)) and configure vertex attributes
  // ------------------------------------------------------------------
  glGenVertexArrays(1, VAO);
  glGenBuffers(1, VBO);
  // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
  glBindVertexArray(*VAO);

  glBindBuffer(GL_ARRAY_BUFFER, *VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0])*3*num_vec3s, vertices, GL_STATIC_DRAW);

  GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
  glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(posAttrib);

  // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
  // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
  glBindVertexArray(0);
}


LineShader CreateLineShader(float *vertices, int num_vertices) {
  LineShader line_shader;
  line_shader.shaderProgram = CreateLineShaderProgram();
  line_shader.num_vertices = num_vertices;

  CreateLineShaderBuffers(line_shader.shaderProgram, &line_shader.VBO, &line_shader.VAO,
                          vertices, num_vertices);
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

  glDrawArrays(mode, 0, line_shader.num_vertices);
}


void UpdateLines(LineShader &line_shader, float *new_verts, int num_vertices) {
  ASSERT(num_vertices == line_shader.num_vertices);

  // make sure they're packed
  //static_assert(sizeof(std::array<glm::vec3, 10>) == sizeof(float)*3*10);

  glBindBuffer(GL_ARRAY_BUFFER, line_shader.VBO);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(float)*3*line_shader.num_vertices,
               new_verts,
               GL_DYNAMIC_DRAW);
}
