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
#include "shader/gridmesh.hpp"
#include "shader/lines.hpp"
#include "shader/colorlines.hpp"

static std::vector<std::vector<ColoredVec3> > AxesLines(const Camera &camera) {
  constexpr glm::vec4 red   = {1, 0, 0, 1};
  constexpr glm::vec4 green = {0, 1, 0, 1};
  constexpr glm::vec4 blue  = {0, 0, 1, 1};
  const glm::vec3 focus_pos = camera.Center();
  const double distance = camera.Distance();
  const double scale = distance * 0.1;

  const glm::vec3 x = {scale,     0,     0};
  const glm::vec3 y = {    0, scale,     0};
  const glm::vec3 z = {    0,     0, scale};

  std::vector<std::vector<ColoredVec3> > segments;
  segments.push_back({{focus_pos,   red}, {focus_pos + x,   red}});
  segments.push_back({{focus_pos, green}, {focus_pos + y, green}});
  segments.push_back({{focus_pos,  blue}, {focus_pos + z,  blue}});

  return segments;
}

int main(int argc __attribute__((unused)),
         char * argv[] __attribute__((unused))) {
  // Boilerplate
  GLFWwindow * const window = OpenglSetup();

  // problem
  Problem<6, 4> problem;
  //Problem<6, 4, 20, 30> problem;

  ProblemVisualization visualization;

  visualization.Update<20, 30>(problem);

  std::chrono::time_point t_start = std::chrono::high_resolution_clock::now();

  ColorLines axes;

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
    (void)model;

    // Camera transformation
    glm::mat4 view = GetViewTransformation();

    // projection transformation
    glm::mat4 proj = GetProjectionTransformation(window);

    visualization.Draw(view, proj);

    // draw axes if we're dragging or rotating
    if (IsDraggingOrRotating()) {
      axes.Update(AxesLines(GetCamera()));
      axes.Draw(view, proj, GL_LINE_STRIP);
    }

    // Swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  return EXIT_SUCCESS;
}
