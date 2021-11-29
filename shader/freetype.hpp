#pragma once

#include <glm/glm.hpp>              // for vec3
#include <GL/glew.h>                // for GLchar, GLuint
#include <string>                   // for string
#include <map>                      // for map

#include "shader/shader.hpp"        // for GLFWwindow, Shader

class Freetype {
public:
  Freetype(int font_size);
  ~Freetype() = default;
  void RenderText(GLFWwindow *window, std::string text, float x, float y, glm::vec3 color);
private:
  Shader shader_;
  GLuint vao_;
  GLuint vbo_;

  // Holds all state information relevant to a character as loaded using FreeType
  struct Character {
    unsigned int TextureID; // ID handle of the glyph texture
    glm::ivec2   Size;      // Size of glyph
    glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
    unsigned int Advance;   // Horizontal offset to advance to next glyph
  };

  std::map<GLchar, Character> characters_;
};
