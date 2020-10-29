#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

GLFWwindow* OpenglSetup();
glm::mat4 GetViewTransformation();
glm::mat4 GetProjectionTransformation(GLFWwindow *window);
