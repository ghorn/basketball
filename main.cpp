#include <cstdio>
#include <cstdlib>
#include <chrono>
#include <string>

#include <eigen3/Eigen/Dense>
#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.hpp"
#include "bspline.hpp"
#include "shader/backboard.hpp"
#include "shader/cat_texture.hpp"

struct GlobalState {
  Camera camera;
  double cursor_rotating_previous_xpos;
  double cursor_rotating_previous_ypos;
  bool cursor_rotating;
  double cursor_panning_previous_xpos;
  double cursor_panning_previous_ypos;
  bool cursor_panning;
};
GlobalState global_state;

static void KeyCallback(GLFWwindow* window,
                        int key,
                        int scancode __attribute__((unused)),
                        int action,
                        int mods __attribute__((unused))) {
  fprintf(stderr, "Key press! %d\n", key);
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
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
  if (global_state.cursor_panning) {
    global_state.camera.Pan(static_cast<float>(xpos - global_state.cursor_panning_previous_xpos),
                            static_cast<float>(ypos - global_state.cursor_panning_previous_ypos));
    global_state.cursor_panning_previous_xpos = xpos;
    global_state.cursor_panning_previous_ypos = ypos;
  }
}

static void MouseButtonCallback(GLFWwindow* window,
                                int button,
                                int action,
                                int mods __attribute__((unused))) {
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
    global_state.cursor_panning = true;

    // set previous position
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    global_state.cursor_panning_previous_xpos = xpos;
    global_state.cursor_panning_previous_ypos = ypos;
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
    global_state.cursor_panning = false;

    // initialize previous position for determinism
    global_state.cursor_panning_previous_xpos = 0;
    global_state.cursor_panning_previous_ypos = 0;
  }

  //fprintf(stderr, "Mouse button pressed: %d %d (%.1f, %.1f)\n", button, action, xpos, ypos);
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

static GLFWwindow* OpenglSetup() {
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
  GLFWwindow* const window = glfwCreateWindow(800, 600, "basketball", nullptr, nullptr);
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

  // Create Context and Load OpenGL Functions
  glfwMakeContextCurrent(window);

  glewExperimental = GL_TRUE;
  glewInit();
  glEnable(GL_DEPTH_TEST);

  fprintf(stderr, "OpenGL %s\n", glGetString(GL_VERSION));
  glfwSwapInterval(1);

  return window;
}

int main(int argc, char * argv[]) {
  Eigen::Matrix<glm::dvec3, 6, 4> ps;
  ps <<
    glm::dvec3(1.0,0, 1.0),glm::dvec3(1.0,3, 1.0),glm::dvec3(1.0,4, 0.0),glm::dvec3(1.0,6, 1.0),
    glm::dvec3(1.5,0, 1.0),glm::dvec3(1.5,3, 1.0),glm::dvec3(1.5,4, 1.0),glm::dvec3(1.5,6, 0.0),
    glm::dvec3(2.0,0,-1.0),glm::dvec3(2.0,3,-2.0),glm::dvec3(2.0,4,-1.0),glm::dvec3(2.0,6,-1.0),
    glm::dvec3(3.0,0, 0.0),glm::dvec3(3.0,3, 0.0),glm::dvec3(3.0,4, 1.0),glm::dvec3(3.0,7, 0.0),
    glm::dvec3(4.0,0, 1.0),glm::dvec3(4.0,3, 4.0),glm::dvec3(4.0,4, 1.0),glm::dvec3(4.0,6, 1.0),
    glm::dvec3(5.0,0, 0.0),glm::dvec3(5.0,3, 0.0),glm::dvec3(5.0,4, 0.0),glm::dvec3(5.0,6, 0.0);

  const Eigen::Matrix<glm::dvec3, 20, 30> interpolated_ps = CubicBSplineSurface<20, 30, 6, 4>(ps);
  (void)interpolated_ps;

  // Parse args
  if (argc != 2) {
    fprintf(stderr, "need one argument: court.jpg location\n");
    exit(EXIT_FAILURE);
  }
  const std::string image_path = argv[1];

  // Boilerplate
  GLFWwindow * const window = OpenglSetup();

  // Backboard
  BackboardShader backboard = CreateBackboardFromMatrix(interpolated_ps);
  // Cat
  CatShader cat_shader = CreateCatShader(image_path);

  std::chrono::time_point t_start = std::chrono::high_resolution_clock::now();
  while (glfwWindowShouldClose(window) == false) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
      glfwSetWindowShouldClose(window, true);
    }

    // Clear the screen to black
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Calculate transformation
    std::chrono::time_point t_now = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();

    // Model transformation
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(
      model,
      time * glm::radians(0.1f * 180.0f),
      glm::vec3(0.0f, 0.0f, 1.0f)
    );

    // Camera transformation
    glm::mat4 view = glm::lookAt(
      global_state.camera.Eye(),
      global_state.camera.Center(),
      glm::vec3(0.0f, 0.0f, -1.0f)
    );

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

    // cat
    DrawCatShader(cat_shader, model, view, proj);

    // backboard
    DrawBackboard(backboard, view, proj);

    // Swap buffers
    glfwSwapBuffers(window);

    glfwPollEvents();
  }

  FreeBackboardGlResources(backboard);
  FreeCatGlResources(cat_shader);

  glfwDestroyWindow(window);
  glfwTerminate();
  return EXIT_SUCCESS;
}
