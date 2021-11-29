#pragma once

#include <GL/glew.h>  // for GLuint
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <string>

GLuint CompileAndLinkVertexFragmentShaderProgram(const std::string &vertex_shader_source,
                                                 const std::string &fragment_shader_source);
