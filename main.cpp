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
#include "problem/backboard.hpp"
#include "problem/problem.hpp"
#include "problem/visualization.hpp"
#include "shader/cat_texture.hpp"
#include "shader/gridmesh.hpp"
#include "shader/lines.hpp"

int main(int argc, char * argv[]) {
  // Parse args
  if (argc != 2) {
    fprintf(stderr, "need one argument: court.jpg location\n");
    exit(EXIT_FAILURE);
  }
  const std::string image_path = argv[1];

  // Boilerplate
  GLFWwindow * const window = OpenglSetup();

  // problem
  Problem<6, 4> problem;
  //Problem<6, 4, 20, 30> problem;

  ProblemVisualization visualization;

  visualization.Update(problem);

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

    visualization.Draw(view, proj);

    // Swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  visualization.FreeResources();
  FreeCatGlResources(cat_shader);

  glfwDestroyWindow(window);
  glfwTerminate();
  return EXIT_SUCCESS;
}
