#include "gridmesh.hpp"

#include <iostream>
#include <vector>

#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <SOIL/SOIL.h>

#include "shader/compile.hpp"
#include "assert.hpp"

const GLchar* gridmeshVertexShaderSource = R"glsl(
  #version 400 core
  layout (location = 0) in vec3 position;
  layout (location = 1) in vec2 texture_coordinate;
  out vec2 texture_coordinate_;
  uniform mat4 view;
  uniform mat4 proj;
  void main()
  {
    texture_coordinate_ = texture_coordinate;
    gl_Position = proj * view * vec4(position, 1.0);
  }
)glsl";

const char *gridmeshFragmentShaderSource = R"glsl(
  #version 400 core
  in vec2 texture_coordinate_;
  uniform sampler2D image_texture;
  out vec4 FragColor;
  void main()
  {
    FragColor = texture(image_texture, texture_coordinate_);
  }
)glsl";


GridmeshShader CreateGridmesh(const std::string &image_path) {
  GridmeshShader gridmesh;
  gridmesh.shaderProgram =
    CompileAndLinkVertexFragmentShaderProgram(gridmeshVertexShaderSource,
                                              gridmeshFragmentShaderSource);

  gridmesh.num_indices = 0;
  gridmesh.vertex_buffer_size = 0;
  gridmesh.index_buffer_size = 0;

  // set up vertex data (and buffer(s)) and configure vertex attributes
  // ------------------------------------------------------------------
  glGenVertexArrays(1, &gridmesh.VAO);
  glGenBuffers(1, &gridmesh.VBO);
  glGenBuffers(1, &gridmesh.EBO);
  // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
  glBindVertexArray(gridmesh.VAO);

  glBindBuffer(GL_ARRAY_BUFFER, gridmesh.VBO);
  glBufferData(GL_ARRAY_BUFFER, gridmesh.vertex_buffer_size, nullptr, GL_DYNAMIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gridmesh.EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, gridmesh.index_buffer_size, nullptr, GL_DYNAMIC_DRAW);

  glVertexAttribPointer(glGetAttribLocation(gridmesh.shaderProgram, "position"),
                        3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(0*sizeof(GLfloat)));
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(glGetAttribLocation(gridmesh.shaderProgram, "texture_coordinate"),
                        2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3*sizeof(GLfloat)));
  glEnableVertexAttribArray(1);

  // load and create a texture
  // -------------------------
  glGenTextures(1, &gridmesh.texture);
  glBindTexture(GL_TEXTURE_2D, gridmesh.texture);

  // set texture wrapping parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  float borderColor[] = {1.0f, 1.0f, 0.0f, 1.0f};
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
  // set texture filtering parameters
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Load from file
  int image_width, image_height;
  unsigned char* image = SOIL_load_image(image_path.c_str(),
                                         &image_width, &image_height, 0, SOIL_LOAD_RGBA);
  if (image == nullptr) {
    fprintf(stderr, "Can't load image %s: %s\n", image_path.c_str(), SOIL_last_result());
    exit(EXIT_FAILURE);
  }
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
  glGenerateMipmap(GL_TEXTURE_2D);
  SOIL_free_image_data(image);

  // set image texture
  glUseProgram(gridmesh.shaderProgram);
  glUniform1i(glGetUniformLocation(gridmesh.shaderProgram, "image_texture"), 0);

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
  // bind textures?
  //glActiveTexture(GL_TEXTURE0);
  //glBindTexture(GL_TEXTURE_2D, gridmesh.texture);

  // render
  glUseProgram(gridmesh.shaderProgram);
  glBindVertexArray(gridmesh.VAO);

  // Set up transformations
  glUniformMatrix4fv(glGetUniformLocation(gridmesh.shaderProgram, "view"),
                     1, GL_FALSE, glm::value_ptr(view));
  glUniformMatrix4fv(glGetUniformLocation(gridmesh.shaderProgram, "proj"),
                     1, GL_FALSE, glm::value_ptr(proj));

  glDrawElements(GL_TRIANGLES, gridmesh.num_indices, GL_UNSIGNED_INT, 0);

  glBindVertexArray(0);
}

void UpdateGridmesh(GridmeshShader &gridmesh,
                    const Eigen::Matrix<glm::vec3, Eigen::Dynamic, Eigen::Dynamic> &grid) {
  const int rows = static_cast<int>(grid.rows()); // readability below
  const int cols = static_cast<int>(grid.cols()); // readability below

  ASSERT(rows > 2);
  ASSERT(cols > 2);

  // Massage the data.
  std::vector<float> vertices;
  for (int ku=0; ku<rows; ku++) {
    for (int kv=0; kv<cols; kv++) {
      const float s = static_cast<float>(ku) / static_cast<float>(rows - 1);
      const float t = static_cast<float>(kv) / static_cast<float>(cols - 1);
      const glm::vec3 &pos = grid(ku, kv);
      vertices.insert(vertices.end(), {pos.x, pos.y, pos.z, s, t});
    }
  }

  std::vector<GLuint> indices;
  for (int ku=0; ku<rows - 1; ku++) {
    for (int kv=0; kv<cols - 1; kv++) {
      int me = ku*cols + kv;
      int right = me + 1;
      int down = (ku + 1)*cols + kv;
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
  const GLint vertex_buffer_size = static_cast<GLint>(sizeof(vertices[0])*vertices.size());
  const GLint index_buffer_size = static_cast<GLint>(sizeof(indices[0])*indices.size());

  glBindBuffer(GL_ARRAY_BUFFER, gridmesh.VBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gridmesh.EBO);

  if (index_buffer_size == gridmesh.index_buffer_size) {
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, index_buffer_size, indices.data());
  } else {
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_size, indices.data(), GL_DYNAMIC_DRAW);
    gridmesh.index_buffer_size = index_buffer_size;
  }

  if (vertex_buffer_size == gridmesh.vertex_buffer_size) {
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_buffer_size, vertices.data());
  } else {
    glBufferData(GL_ARRAY_BUFFER, vertex_buffer_size, vertices.data(), GL_DYNAMIC_DRAW);
    gridmesh.vertex_buffer_size = vertex_buffer_size;
  }

  if (num_indices != gridmesh.num_indices) {
    gridmesh.num_indices = num_indices;
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
