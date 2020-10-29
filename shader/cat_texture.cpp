#include "cat_texture.hpp"

#include <iostream>

#include <SOIL/SOIL.h>
#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

// Shader sources
const GLchar* vertexSource = R"glsl(
    #version 150 core
    in vec2 position;
    in vec3 color;
    in vec2 texcoord;
    out vec3 Color;
    out vec2 Texcoord;
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 proj;
    void main()
    {
        Color = color;
        Texcoord = texcoord;
        gl_Position = proj * view * model * vec4(position, 0.0, 1.0);
    }
)glsl";
const GLchar* fragmentSource = R"glsl(
    #version 150 core
    in vec3 Color;
    in vec2 Texcoord;
    out vec4 outColor;
    uniform sampler2D texKitten;
    uniform sampler2D texPuppy;
    void main()
    {
        outColor = mix(texture(texKitten, Texcoord), texture(texPuppy, Texcoord), 0.5);
    }
)glsl";

CatShader CreateCatShader(const std::string &image_path) {
  CatShader cat_shader;

  // Create Vertex Array Object
  glGenVertexArrays(1, &cat_shader.vao);
  glBindVertexArray(cat_shader.vao);

  // Create a Vertex Buffer Object and copy the vertex data to it
  glGenBuffers(1, &cat_shader.vbo);

  GLfloat vertices[] = {
  //  Position      Color             Texcoords
    -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // Top-left
     0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // Top-right
     0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // Bottom-right
    -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f  // Bottom-left
  };

  glBindBuffer(GL_ARRAY_BUFFER, cat_shader.vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // Create an element array
  glGenBuffers(1, &cat_shader.ebo);

  GLuint elements[] = {
    0, 1, 2,
    2, 3, 0
  };

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cat_shader.ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

  // Create and compile the vertex shader
  cat_shader.vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(cat_shader.vertexShader, 1, &vertexSource, NULL);
  glCompileShader(cat_shader.vertexShader);

  // Create and compile the fragment shader
  cat_shader.fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(cat_shader.fragmentShader, 1, &fragmentSource, NULL);
  glCompileShader(cat_shader.fragmentShader);

  // Link the vertex and fragment shader into a shader program
  cat_shader.shaderProgram = glCreateProgram();
  glAttachShader(cat_shader.shaderProgram, cat_shader.vertexShader);
  glAttachShader(cat_shader.shaderProgram, cat_shader.fragmentShader);
  glBindFragDataLocation(cat_shader.shaderProgram, 0, "outColor");
  glLinkProgram(cat_shader.shaderProgram);
  glUseProgram(cat_shader.shaderProgram);

  // Specify the layout of the vertex data
  GLint posAttrib = glGetAttribLocation(cat_shader.shaderProgram, "position");
  glEnableVertexAttribArray(posAttrib);
  glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), 0);

  GLint colAttrib = glGetAttribLocation(cat_shader.shaderProgram, "color");
  glEnableVertexAttribArray(colAttrib);
  glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));

  GLint texAttrib = glGetAttribLocation(cat_shader.shaderProgram, "texcoord");
  glEnableVertexAttribArray(texAttrib);
  glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(5 * sizeof(GLfloat)));

  // Load textures
  glGenTextures(2, cat_shader.textures);

  int width, height;
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, cat_shader.textures[0]);
  unsigned char* image = SOIL_load_image("sample.png", &width, &height, 0, SOIL_LOAD_RGB);
  //unsigned char* image = SOIL_load_image(image_path.c_str(), &width, &height, 0, SOIL_LOAD_RGB);
  if (image == nullptr) {
    fprintf(stderr, "Can't load image %s: %s\n", image_path.c_str(), SOIL_last_result());
    exit(EXIT_FAILURE);
  }
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
  SOIL_free_image_data(image);
  glUniform1i(glGetUniformLocation(cat_shader.shaderProgram, "texKitten"), 0);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, cat_shader.textures[1]);
      image = SOIL_load_image("sample2.png", &width, &height, 0, SOIL_LOAD_RGB);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
      SOIL_free_image_data(image);
  glUniform1i(glGetUniformLocation(cat_shader.shaderProgram, "texPuppy"), 1);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Set up transformations
  cat_shader.uniModel = glGetUniformLocation(cat_shader.shaderProgram, "model");
  cat_shader.uniView = glGetUniformLocation(cat_shader.shaderProgram, "view");
  cat_shader.uniProj = glGetUniformLocation(cat_shader.shaderProgram, "proj");

  return cat_shader;
}

void DrawCatShader(const CatShader &cat_shader,
                   const glm::mat4 &model,
                   const glm::mat4 &view,
                   const glm::mat4 &proj) {
  glUseProgram(cat_shader.shaderProgram);
  glBindVertexArray(cat_shader.vao);

  glUniformMatrix4fv(cat_shader.uniModel, 1, GL_FALSE, glm::value_ptr(model));
  glUniformMatrix4fv(cat_shader.uniView, 1, GL_FALSE, glm::value_ptr(view));
  glUniformMatrix4fv(cat_shader.uniProj, 1, GL_FALSE, glm::value_ptr(proj));
  // Draw a rectangle from the 2 triangles using 6 indices
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void FreeCatGlResources(const CatShader &cat_shader) {
  glDeleteTextures(2, cat_shader.textures);

  glDeleteProgram(cat_shader.shaderProgram);
  glDeleteShader(cat_shader.fragmentShader);
  glDeleteShader(cat_shader.vertexShader);

  glDeleteBuffers(1, &cat_shader.ebo);
  glDeleteBuffers(1, &cat_shader.vbo);

  glDeleteVertexArrays(1, &cat_shader.vao);
}
