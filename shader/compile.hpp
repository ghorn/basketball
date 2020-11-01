#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

GLuint CompileAndLinkVertexFragmentShaderProgram(const std::string &vertex_shader_source,
                                                 const std::string &fragment_shader_source);
