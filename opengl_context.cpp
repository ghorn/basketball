#include "opengl_context.hpp"

#include <iostream>

#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.hpp"
#include "gl_error.hpp"

struct GlobalState {
  Camera camera;
  bool cursor_rotating;
  double cursor_rotating_previous_xpos;
  double cursor_rotating_previous_ypos;
  bool cursor_xy_translating;
  double cursor_xy_translating_previous_xpos;
  double cursor_xy_translating_previous_ypos;
  bool cursor_z_translating;
  double cursor_z_translating_previous_xpos;
  double cursor_z_translating_previous_ypos;
};
GlobalState global_state;

bool IsDraggingOrRotating() {
  return global_state.cursor_rotating || global_state.cursor_xy_translating || global_state.cursor_z_translating;
}

const Camera &GetCamera() {
  return global_state.camera;
}

static void KeyCallback(GLFWwindow* window,
                        int key,
                        int scancode,
                        int action,
                        int mods __attribute__((unused))) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  } else {
    fprintf(stderr, "Key press! %s (%d)\n", glfwGetKeyName(key, scancode), key);
  }
}

static void WindowSizeCallback(GLFWwindow* window __attribute__((unused)),
                               int width,
                               int height) {
  glViewport(0, 0, width, height);
}

static void CursorPositionCallback(GLFWwindow* window __attribute__((unused)),
                                   double xpos,
                                   double ypos) {
  if (global_state.cursor_rotating) {
    global_state.camera.Rotate(static_cast<float>(xpos - global_state.cursor_rotating_previous_xpos),
                               static_cast<float>(ypos - global_state.cursor_rotating_previous_ypos));
    global_state.cursor_rotating_previous_xpos = xpos;
    global_state.cursor_rotating_previous_ypos = ypos;
  }
  if (global_state.cursor_xy_translating) {
    global_state.camera.TranslateXy(static_cast<float>(xpos - global_state.cursor_xy_translating_previous_xpos),
                                    static_cast<float>(ypos - global_state.cursor_xy_translating_previous_ypos));
    global_state.cursor_xy_translating_previous_xpos = xpos;
    global_state.cursor_xy_translating_previous_ypos = ypos;
  }
  if (global_state.cursor_z_translating) {
    global_state.camera.TranslateZ(static_cast<float>(xpos - global_state.cursor_z_translating_previous_xpos),
                                    static_cast<float>(ypos - global_state.cursor_z_translating_previous_ypos));
    global_state.cursor_z_translating_previous_xpos = xpos;
    global_state.cursor_z_translating_previous_ypos = ypos;
  }
}

static void MouseButtonCallback(GLFWwindow* window,
                                int button,
                                int action,
                                int mods __attribute__((unused))) {
  //fprintf(stderr, "Mouse button pressed: %d %d\n", button, action);

  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    // emable drag state
    global_state.cursor_rotating = true;

    // set previous position
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    global_state.cursor_rotating_previous_xpos = xpos;
    global_state.cursor_rotating_previous_ypos = ypos;
  }

  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
    // emable drag state
    global_state.cursor_xy_translating = true;

    // set previous position
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    global_state.cursor_xy_translating_previous_xpos = xpos;
    global_state.cursor_xy_translating_previous_ypos = ypos;
  }

  if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS) {
    // emable drag state
    global_state.cursor_z_translating = true;

    // set previous position
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    global_state.cursor_z_translating_previous_xpos = xpos;
    global_state.cursor_z_translating_previous_ypos = ypos;
  }

  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
    // disable drag state
    global_state.cursor_rotating = false;

    // initialize previous position for determinism
    global_state.cursor_rotating_previous_xpos = 0;
    global_state.cursor_rotating_previous_ypos = 0;
  }

  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
    // disable drag state
    global_state.cursor_xy_translating = false;

    // initialize previous position for determinism
    global_state.cursor_xy_translating_previous_xpos = 0;
    global_state.cursor_xy_translating_previous_ypos = 0;
  }

  if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE) {
    // disable drag state
    global_state.cursor_z_translating = false;

    // initialize previous position for determinism
    global_state.cursor_z_translating_previous_xpos = 0;
    global_state.cursor_z_translating_previous_ypos = 0;
  }
}

static void ScrollCallback(GLFWwindow* window __attribute__((unused)),
                           double xoffset __attribute__((unused)),
                           double yoffset) {
  global_state.camera.Scroll(static_cast<float>(yoffset));
}

static void ErrorCallback(int error, const char* description)
{
  fprintf(stderr, "Error (%d): %s\n", error, description);
}

GLFWwindow* OpenglSetup() {
  glfwSetErrorCallback(ErrorCallback);

  // Load GLFW and Create a Window
  if (!glfwInit()) {
    fprintf(stderr, "Failed to initialize glfw");
    exit(EXIT_FAILURE);
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

  // Create window.
  GLFWwindow* const window = glfwCreateWindow(0.7*1920, 0.7*1080, "basketball", nullptr, nullptr);
  if (window == nullptr) {
    fprintf(stderr, "Failed to Create OpenGL Context");
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  // Set callbacks.
  glfwSetKeyCallback(window, KeyCallback);
  glfwSetCursorPosCallback(window, CursorPositionCallback);
  glfwSetMouseButtonCallback(window, MouseButtonCallback);
  glfwSetScrollCallback(window, ScrollCallback);
  glfwSetWindowSizeCallback(window, WindowSizeCallback);

  // Create Context and Load OpenGL Functions
  glfwMakeContextCurrent(window);

  glewExperimental = GL_TRUE;
  glewInit();
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_PROGRAM_POINT_SIZE);

  // blending
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  //glBlendFunc(GL_SRC_ALPHA, GL_SRC_ALPHA_SATURATE);

  // pretty
  glEnable(GL_LINE_SMOOTH);
  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
  glEnable(GL_POLYGON_SMOOTH);
  glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

  // Debugging
  glEnable(GL_DEBUG_OUTPUT);
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  glDebugMessageCallback(GlDebugOutput, nullptr);
  glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

  fprintf(stderr, "OpenGL %s\n", glGetString(GL_VERSION));
  glfwSwapInterval(1);

  return window;
}

glm::mat4 GetViewTransformation() {
  return glm::lookAt(
      global_state.camera.Eye(),
      global_state.camera.Center(),
      glm::vec3(0.0f, 0.0f, -1.0f));
}

glm::mat4 GetProjectionTransformation(GLFWwindow *window) {
  // projection transformation
  const float min_clip = 1e-3f;
  const float max_clip = 1e4f;
  int width, height;
  glfwGetWindowSize(window, &width, &height);
  width = std::max(width, 1);
  height = std::max(height, 1);
  glm::mat4 proj = glm::perspective(glm::radians(45.0f),
                                    static_cast<float>(width)/static_cast<float>(height),
                                    min_clip,
                                    max_clip);
  return proj;
}
