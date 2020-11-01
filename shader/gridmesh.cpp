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

Gridmesh::Gridmesh(const std::string &image_path) {
  shader_ =
    CompileAndLinkVertexFragmentShaderProgram("shader/gridmesh.vs", "shader/gridmesh.fs");

  num_indices_ = 0;
  vertex_buffer_size_ = 0;
  index_buffer_size_ = 0;

  // set up vertex data (and buffer(s)) and configure vertex attributes
  // ------------------------------------------------------------------
  glGenVertexArrays(1, &vao_);
  glGenBuffers(1, &vbo_);
  glGenBuffers(1, &ebo_);
  // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
  glBindVertexArray(vao_);

  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferData(GL_ARRAY_BUFFER, vertex_buffer_size_, nullptr, GL_DYNAMIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_size_, nullptr, GL_DYNAMIC_DRAW);

  glVertexAttribPointer(glGetAttribLocation(shader_, "position"),
                        3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(0*sizeof(GLfloat)));
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(glGetAttribLocation(shader_, "texture_coordinate"),
                        2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3*sizeof(GLfloat)));
  glEnableVertexAttribArray(1);

  // load and create a texture
  // -------------------------
  glGenTextures(1, &texture_);
  glBindTexture(GL_TEXTURE_2D, texture_);

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
  glUseProgram(shader_);
  glUniform1i(glGetUniformLocation(shader_, "image_texture"), 0);

  // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
  //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
  // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
  glBindVertexArray(0);
}

void Gridmesh::Draw(const glm::mat4 &view, const glm::mat4 &proj) {
  // bind textures?
  //glActiveTexture(GL_TEXTURE0);
  //glBindTexture(GL_TEXTURE_2D, texture_);

  // render
  glUseProgram(shader_);
  glBindVertexArray(vao_);

  // Set up transformations
  glUniformMatrix4fv(glGetUniformLocation(shader_, "view"),
                     1, GL_FALSE, glm::value_ptr(view));
  glUniformMatrix4fv(glGetUniformLocation(shader_, "proj"),
                     1, GL_FALSE, glm::value_ptr(proj));

  glDrawElements(GL_TRIANGLES, num_indices_, GL_UNSIGNED_INT, 0);

  glBindVertexArray(0);
}

void Gridmesh::Update(const Eigen::Matrix<glm::vec3, Eigen::Dynamic, Eigen::Dynamic> &grid) {
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

  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);

  if (index_buffer_size == index_buffer_size_) {
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, index_buffer_size, indices.data());
  } else {
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_size, indices.data(), GL_DYNAMIC_DRAW);
    index_buffer_size_ = index_buffer_size;
  }

  if (vertex_buffer_size == vertex_buffer_size_) {
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_buffer_size, vertices.data());
  } else {
    glBufferData(GL_ARRAY_BUFFER, vertex_buffer_size, vertices.data(), GL_DYNAMIC_DRAW);
    vertex_buffer_size_ = vertex_buffer_size;
  }

  if (num_indices != num_indices_) {
    num_indices_ = num_indices;
  }

  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

Gridmesh::~Gridmesh() {
  // de-allocate all resources once they've outlived their purpose:
  // ------------------------------------------------------------------------
  glDeleteVertexArrays(1, &vao_);
  glDeleteBuffers(1, &vbo_);
  glDeleteBuffers(1, &ebo_);
  glDeleteProgram(shader_);
}
