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
#include "opengl_context.hpp"
#include "shader/backboard.hpp"
#include "shader/cat_texture.hpp"

int main(int argc, char * argv[]) {
  Eigen::Matrix<glm::dvec3, 6, 4> ps;
  ps <<
    glm::dvec3(1.0,0, 1.0),glm::dvec3(1.0,3, 1.0),glm::dvec3(1.0,4, 0.0),glm::dvec3(1.0,6, 1.0),
    glm::dvec3(1.5,0, 1.0),glm::dvec3(1.5,3, 1.0),glm::dvec3(1.5,4, 1.0),glm::dvec3(1.5,6, 0.0),
    glm::dvec3(2.0,0,-1.0),glm::dvec3(2.0,3,-2.0),glm::dvec3(2.0,4,-1.0),glm::dvec3(2.0,6,-1.0),
    glm::dvec3(3.0,0, 0.0),glm::dvec3(3.0,3, 0.0),glm::dvec3(3.0,4, 1.0),glm::dvec3(3.0,7, 0.0),
    glm::dvec3(4.0,0, 1.0),glm::dvec3(4.0,3, 4.0),glm::dvec3(4.0,4, 1.0),glm::dvec3(4.0,6, 1.0),
    glm::dvec3(5.0,0, 0.0),glm::dvec3(5.0,3, 0.0),glm::dvec3(5.0,4, 0.0),glm::dvec3(5.0,6, 0.0);

  const Eigen::Matrix<glm::dvec3, 20, 30> interpolated_ps = ClampedCubicBSplineSurface<20, 30, 6, 4>(ps);

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
    // Clear the screen to black
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Calculate transformation
    std::chrono::time_point t_now = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();

    // Model transformation
    glm::mat4 model = glm::rotate(
      glm::mat4(1.0f),
      time * glm::radians(0.1f * 180.0f),
      glm::vec3(0.0f, 0.0f, 1.0f)
    );

    // Camera transformation
    glm::mat4 view = GetViewTransformation();

    // projection transformation
    glm::mat4 proj = GetProjectionTransformation(window);

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
