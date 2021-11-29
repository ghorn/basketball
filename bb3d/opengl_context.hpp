#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "bb3d/camera.hpp"

GLFWwindow* OpenglSetup();
bool IsDraggingOrRotating();
const Camera &GetCamera();
glm::mat4 GetViewTransformation();
glm::mat4 GetProjectionTransformation(GLFWwindow *window);
bool KeypressQueueEmpty();
int PopKeypressQueue();
