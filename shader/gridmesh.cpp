#include "gridmesh.hpp"

#include <iostream>
#include <vector>

#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>


const GLchar* vertexShaderSource = R"glsl(
  #version 400 core
  layout (location = 0) in vec3 aPos;
  uniform mat4 view;
  uniform mat4 proj;
  void main()
  {
   gl_Position = proj * view * vec4(aPos, 1.0);
  }
)glsl";

const char *fragmentShaderSource = R"glsl(
  #version 400 core
  out vec4 FragColor;
  void main()
  {
   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
  }
)glsl";

GLint CreateGridmeshShaderProgram() {
  // build and compile our shader program
  // ------------------------------------
  // vertex shader
  GLint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
  glCompileShader(vertexShader);
  // check for shader compile errors
  GLint success;
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if (!success)
  {
    char infoLog[512];
    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
  }
  // fragment shader
  GLint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
  glCompileShader(fragmentShader);
  // check for shader compile errors
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  if (!success)
  {
    char infoLog[512];
    glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
    std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
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
  }
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  return shaderProgram;
}

static void CreateGridmeshBuffers(GLuint *VBO, GLuint *VAO, GLuint *EBO,
                                   float *vertices, int num_vec3s,
                                   unsigned int *indices, int num_indices) {
  // set up vertex data (and buffer(s)) and configure vertex attributes
  // ------------------------------------------------------------------
  //float vertices[] = {
  //   0.5f,  0.5f, 0.0f,  // top right
  //   0.5f, -0.5f, 0.0f,  // bottom right
  //  -0.5f, -0.5f, 0.0f,  // bottom left
  //  -0.5f,  0.5f, 0.0f   // top left 
  //};
  //unsigned int indices[] = {  // note that we start from 0!
  //  0, 1, 3,  // first Triangle
  //  1, 2, 3   // second Triangle
  //};
  glGenVertexArrays(1, VAO);
  glGenBuffers(1, VBO);
  glGenBuffers(1, EBO);
  // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
  glBindVertexArray(*VAO);

  glBindBuffer(GL_ARRAY_BUFFER, *VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0])*3*num_vec3s, vertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0])*num_indices, indices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
  glBindBuffer(GL_ARRAY_BUFFER, 0); 

  // remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
  //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
  // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
  glBindVertexArray(0); 
}

GridmeshShader CreateGridmesh(float *vertices, int rows, int cols) {
  GridmeshShader gridmesh;
  gridmesh.shaderProgram = CreateGridmeshShaderProgram();

  int num_vertices = rows*cols;
  std::vector<GLuint> indices;
  for (int kx=0; kx<rows - 1; kx++) {
    for (int ky=0; ky<cols - 1; ky++) {
      int me = kx*cols + ky;
      int right = me + 1;
      int down = (kx + 1)*cols + ky;
      int corner = down + 1;
      // triangle 1
      indices.push_back(me);
      indices.push_back(right);
      indices.push_back(corner);
      // triangle 2
      indices.push_back(me);
      indices.push_back(corner);
      indices.push_back(down);
    }
  }
  gridmesh.num_indices = static_cast<GLint>(indices.size());
  CreateGridmeshBuffers(&gridmesh.VBO, &gridmesh.VAO, &gridmesh.EBO, vertices, num_vertices, indices.data(), gridmesh.num_indices);
  return gridmesh;
}

void DrawGridmesh(const GridmeshShader &gridmesh,
                   const glm::mat4  &view,
                   const glm::mat4  &proj) {
  // draw triangle
  glUseProgram(gridmesh.shaderProgram);
  glBindVertexArray(gridmesh.VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized

  // Set up transformations
  GLint uniView = glGetUniformLocation(gridmesh.shaderProgram, "view");
  GLint uniProj = glGetUniformLocation(gridmesh.shaderProgram, "proj");
  glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));
  glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));

  //glDrawArrays(GL_TRIANGLES, 0, 6);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glDrawElements(GL_TRIANGLES, gridmesh.num_indices, GL_UNSIGNED_INT, 0);
  // glBindVertexArray(0); // no need to unbind it every time 
}

void FreeGridmeshGlResources(const GridmeshShader &gridmesh) {
  // optional: de-allocate all resources once they've outlived their purpose:
  // ------------------------------------------------------------------------
  glDeleteVertexArrays(1, &gridmesh.VAO);
  glDeleteBuffers(1, &gridmesh.VBO);
  glDeleteBuffers(1, &gridmesh.EBO);
  glDeleteProgram(gridmesh.shaderProgram);
}
