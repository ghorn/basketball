#pragma once

#include <iostream>

#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

GLenum GlCheckError_(const char *file, int line);
#define GL_CHECK_ERROR() glCheckError_(__FILE__, __LINE__)

void GlDebugOutput(GLenum source, 
                   GLenum type, 
                   unsigned int id, 
                   GLenum severity, 
                   GLsizei length, 
                   const char *message, 
                   const void *userParam);
