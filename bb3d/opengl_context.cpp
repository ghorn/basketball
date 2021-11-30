#include "bb3d/opengl_context.hpp"

#include <GL/glew.h>                     // for glEnable, GL_TRUE, GL_DONT_CARE, glGetString
#include <algorithm>                     // for max
#include <cmath>
#include <cstdio>                        // for fprintf, stderr
#include <cstdlib>                       // for exit, EXIT_FAILURE
#include <iostream>
#include <queue>                         // for queue
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>                  // for glfwWindowHint, GLFWwindow, glfwGetCursorPos
#include <glm/glm.hpp>                   // for vec3, mat4, radians, vec<>::(anonymous)
#include <glm/gtc/matrix_transform.hpp>  // for lookAt, ortho, perspective

#include "bb3d/assert.hpp"
#include "bb3d/camera.hpp"                    // for Camera
#include "bb3d/gl_error.hpp"                  // for GlDebugOutput


namespace bb3d {
static GLFWwindow* OpenglSetup(WindowState *window_state);

Window::Window(std::unique_ptr<WindowState> window_state) : window_state_(std::move(window_state)) {
  glfw_window = OpenglSetup(window_state_.get());
};

Window::~Window() {
  glfwDestroyWindow(glfw_window);
  glfwTerminate();
};

bool Window::ShouldClose() {
  return glfwWindowShouldClose(glfw_window) != 0;
}

void Window::SwapBuffers() {
  glfwSwapBuffers(glfw_window);
}

void Window::PollEvents() {
  glfwPollEvents();
}


Window::Size Window::GetSize() const {
  Window::Size window_size{};
  glfwGetWindowSize(glfw_window, &window_size.width, &window_size.height);
  return window_size;
}

const Camera & WindowState::GetCamera() const {
  return camera;
}

static void KeyCallback(GLFWwindow* glfw_window,
                        int key,
                        int scancode,
                        int action,
                        int mods __attribute__((unused))) {
  WindowState &window_state = *reinterpret_cast<WindowState *>(glfwGetWindowUserPointer(glfw_window));
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(glfw_window, GLFW_TRUE);
  } else if (action == GLFW_PRESS) {
    window_state.keypress_queue.push(key);
    fprintf(stderr, "Key press! %s (%d)\n", glfwGetKeyName(key, scancode), key);
  }
}

static void WindowSizeCallback(GLFWwindow* window __attribute__((unused)),
                               int width,
                               int height) {
  glViewport(0, 0, width, height);
}

static void CursorPositionCallback(GLFWwindow* glfw_window,
                                   double xpos,
                                   double ypos) {
  WindowState &window_state = *reinterpret_cast<WindowState *>(glfwGetWindowUserPointer(glfw_window));
  if (window_state.mouse_handler.cursor_rotating) {
    window_state.camera.Rotate(static_cast<float>(xpos - window_state.mouse_handler.cursor_rotating_previous_xpos),
                               static_cast<float>(ypos - window_state.mouse_handler.cursor_rotating_previous_ypos));
    window_state.mouse_handler.cursor_rotating_previous_xpos = xpos;
    window_state.mouse_handler.cursor_rotating_previous_ypos = ypos;
  }
  if (window_state.mouse_handler.cursor_xy_translating) {
    window_state.camera.TranslateXy(static_cast<float>(xpos - window_state.mouse_handler.cursor_xy_translating_previous_xpos),
                                    static_cast<float>(ypos - window_state.mouse_handler.cursor_xy_translating_previous_ypos));
    window_state.mouse_handler.cursor_xy_translating_previous_xpos = xpos;
    window_state.mouse_handler.cursor_xy_translating_previous_ypos = ypos;
  }
  if (window_state.mouse_handler.cursor_z_translating) {
    window_state.camera.TranslateZ(static_cast<float>(xpos - window_state.mouse_handler.cursor_z_translating_previous_xpos),
                                    static_cast<float>(ypos - window_state.mouse_handler.cursor_z_translating_previous_ypos));
    window_state.mouse_handler.cursor_z_translating_previous_xpos = xpos;
    window_state.mouse_handler.cursor_z_translating_previous_ypos = ypos;
  }
}

static void DescribeNewCameraFocus(const Camera &camera) {
  const glm::vec3 focus_position = camera.Center();
  fprintf(stderr, "Camera focus moved to {%.1f, %.1f, %.1f}\n",
          focus_position.x,
          focus_position.y,
          focus_position.z);
}

static void MouseButtonCallback(GLFWwindow* glfw_window,
                                int button,
                                int action,
                                int mods __attribute__((unused))) {
  WindowState &window_state = *reinterpret_cast<WindowState *>(glfwGetWindowUserPointer(glfw_window));

  //fprintf(stderr, "Mouse button pressed: %d %d\n", button, action);

  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    // emable drag state
    window_state.mouse_handler.cursor_rotating = true;

    // set previous position
    double xpos{};
    double ypos{};
    glfwGetCursorPos(glfw_window, &xpos, &ypos);
    window_state.mouse_handler.cursor_rotating_previous_xpos = xpos;
    window_state.mouse_handler.cursor_rotating_previous_ypos = ypos;
  }

  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
    // emable drag state
    window_state.mouse_handler.cursor_xy_translating = true;

    // set previous position
    double xpos{};
    double ypos{};
    glfwGetCursorPos(glfw_window, &xpos, &ypos);
    window_state.mouse_handler.cursor_xy_translating_previous_xpos = xpos;
    window_state.mouse_handler.cursor_xy_translating_previous_ypos = ypos;
  }

  if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS) {
    // emable drag state
    window_state.mouse_handler.cursor_z_translating = true;

    // set previous position
    double xpos{};
    double ypos{};
    glfwGetCursorPos(glfw_window, &xpos, &ypos);
    window_state.mouse_handler.cursor_z_translating_previous_xpos = xpos;
    window_state.mouse_handler.cursor_z_translating_previous_ypos = ypos;
  }

  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
    // disable drag state
    window_state.mouse_handler.cursor_rotating = false;

    // initialize previous position for determinism
    window_state.mouse_handler.cursor_rotating_previous_xpos = 0;
    window_state.mouse_handler.cursor_rotating_previous_ypos = 0;
  }

  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
    // disable drag state
    window_state.mouse_handler.cursor_xy_translating = false;

    // initialize previous position for determinism
    window_state.mouse_handler.cursor_xy_translating_previous_xpos = 0;
    window_state.mouse_handler.cursor_xy_translating_previous_ypos = 0;
    DescribeNewCameraFocus(window_state.camera);
  }

  if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE) {
    // disable drag state
    window_state.mouse_handler.cursor_z_translating = false;

    // initialize previous position for determinism
    window_state.mouse_handler.cursor_z_translating_previous_xpos = 0;
    window_state.mouse_handler.cursor_z_translating_previous_ypos = 0;
    DescribeNewCameraFocus(window_state.camera);
  }
}

static void ScrollCallback(GLFWwindow* glfw_window,
                           double xoffset __attribute__((unused)),
                           double yoffset) {
  WindowState &window_state = *reinterpret_cast<WindowState *>(glfwGetWindowUserPointer(glfw_window));
  window_state.camera.Scroll(static_cast<float>(yoffset));
}

static void ErrorCallback(int error, const char* description)
{
  fprintf(stderr, "Error (%d): %s\n", error, description);
}

static GLFWwindow* OpenglSetup(WindowState *window_state) {
  glfwSetErrorCallback(ErrorCallback);

  // Load GLFW and Create a Window
  if (glfwInit() == 0) {
    fprintf(stderr, "Failed to initialize glfw");
    exit_thread_safe(EXIT_FAILURE);
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

  // Create window.
  GLFWwindow* const window = glfwCreateWindow(0.7*1920, 0.7*1080, "bb3d", nullptr, nullptr);

  if (window == nullptr) {
    fprintf(stderr, "Failed to Create OpenGL Context");
    glfwTerminate();
    exit_thread_safe(EXIT_FAILURE);
  }

  // Set user pointer.
  glfwSetWindowUserPointer(window, window_state);

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
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_PROGRAM_POINT_SIZE);

  // Debugging
  glEnable(GL_DEBUG_OUTPUT);
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  glDebugMessageCallback(GlDebugOutput, nullptr);
  glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

  fprintf(stderr, "OpenGL %s\n", glGetString(GL_VERSION));
  glfwSwapInterval(1);

  return window;
}

glm::mat4 WindowState::GetViewTransformation() const {
  return glm::lookAt(
    camera.Eye(),
    camera.Center(),
    glm::vec3(0.0f, 0.0f, -1.0f));
}

glm::mat4 Window::GetProjectionTransformation() const {
  const float min_clip = 1e-3f;
  const float max_clip = 1e4f;
  Window::Size window_size = GetSize();
  window_size.width =  std::max(window_size.width, 1);
  window_size.height = std::max(window_size.height, 1);
  const float aspect_ratio = static_cast<float>(window_size.width)/static_cast<float>(window_size.height);
  return glm::perspective(glm::radians(45.0f),
                          aspect_ratio,
                          min_clip,
                          max_clip);
}

glm::mat4 Window::GetOrthographicProjection() const {
  // projection transformation
  Window::Size window_size = GetSize();
  glfwGetWindowSize(glfw_window, &window_size.width, &window_size.height);
  window_size.width = std::max(window_size.width, 1);
  window_size.height = std::max(window_size.height, 1);
  return glm::ortho(0.0f, static_cast<float>(window_size.width),
                    0.0f, static_cast<float>(window_size.height));
}

bool WindowState::IsDraggingOrRotating() const {
  return mouse_handler.cursor_rotating || mouse_handler.cursor_xy_translating || mouse_handler.cursor_z_translating;
}

bool WindowState::KeypressQueueEmpty() const {
  return keypress_queue.empty();
}
int WindowState::PopKeypressQueue() {
  const int next = keypress_queue.front();
  keypress_queue.pop();
  return next;
}

}; //  namespace bb3d
