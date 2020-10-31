#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

GLuint CompileAndLinkVertexFragmentShaderProgram(const char * const vertex_shader_source,
                                                 const char * const fragment_shader_source);
