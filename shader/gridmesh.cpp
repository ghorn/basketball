#include "gridmesh.hpp"

#include <iostream>
#include <vector>

#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

#include "shader/compile.hpp"

const GLchar* gridmeshVertexShaderSource = R"glsl(
  #version 400 core
  layout (location = 0) in vec3 aPos;
  uniform mat4 view;
  uniform mat4 proj;
  void main()
  {
   gl_Position = proj * view * vec4(aPos, 1.0);
  }
)glsl";

const char *gridmeshFragmentShaderSource = R"glsl(
  #version 400 core
  out vec4 FragColor;
  void main()
  {
   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
  }
)glsl";


GridmeshShader CreateGridmesh() {
  GridmeshShader gridmesh;
  gridmesh.shaderProgram =
    CompileAndLinkVertexFragmentShaderProgram(gridmeshVertexShaderSource,
                                              gridmeshFragmentShaderSource);

  gridmesh.current_num_indices = 0;

  // set up vertex data (and buffer(s)) and configure vertex attributes
  // ------------------------------------------------------------------
  glGenVertexArrays(1, &gridmesh.VAO);
  glGenBuffers(1, &gridmesh.VBO);
  glGenBuffers(1, &gridmesh.EBO);
  // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
  glBindVertexArray(gridmesh.VAO);

  glBindBuffer(GL_ARRAY_BUFFER, gridmesh.VBO);
  glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gridmesh.EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
  //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
  // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
  glBindVertexArray(0);


  return gridmesh;
}

void DrawGridmesh(const GridmeshShader &gridmesh,
                   const glm::mat4 &view,
                   const glm::mat4 &proj) {
  // draw triangle
  glUseProgram(gridmesh.shaderProgram);
  glBindVertexArray(gridmesh.VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized

  // Set up transformations
  GLint uniView = glGetUniformLocation(gridmesh.shaderProgram, "view");
  GLint uniProj = glGetUniformLocation(gridmesh.shaderProgram, "proj");
  glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));
  glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glDrawElements(GL_TRIANGLES, gridmesh.current_num_indices, GL_UNSIGNED_INT, 0);
  // glBindVertexArray(0); // no need to unbind it every time
}

void UpdateGridmesh(GridmeshShader &gridmesh,
                    float *vertices, int rows, int cols) {
  // Massage the data.
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

  const GLint num_indices = static_cast<GLint>(indices.size());
  const GLint vertex_buffer_size = static_cast<GLint>(3*sizeof(vertices[0])*num_vertices);
  const GLint index_buffer_size = static_cast<GLint>(sizeof(indices[0])*num_indices);

  glBindBuffer(GL_ARRAY_BUFFER, gridmesh.VBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gridmesh.EBO);

  if (num_indices == gridmesh.current_num_indices) {
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_buffer_size, vertices);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, index_buffer_size, indices.data());
  } else {
    glBufferData(GL_ARRAY_BUFFER, vertex_buffer_size, vertices, GL_DYNAMIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_size, indices.data(), GL_DYNAMIC_DRAW);
    gridmesh.current_num_indices = num_indices;
  }

  glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void FreeGridmeshGlResources(const GridmeshShader &gridmesh) {
  // optional: de-allocate all resources once they've outlived their purpose:
  // ------------------------------------------------------------------------
  glDeleteVertexArrays(1, &gridmesh.VAO);
  glDeleteBuffers(1, &gridmesh.VBO);
  glDeleteBuffers(1, &gridmesh.EBO);
  glDeleteProgram(gridmesh.shaderProgram);
}
