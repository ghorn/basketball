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
  const glm::dvec3 shot_point(4, 0.5, 1);
  const std::vector<Shot> shots = problem.ComputeShots<10, 15>(shot_point);

  // Gridmesh
  GridmeshShader gridmesh = CreateGridmeshFromMatrix(problem.backboard_.Interpolate<20, 30>());

  // Line rendering
  std::vector<float> line;
  for (const Shot &shot : shots) {
    std::array<glm::vec3, 256> shot_arc;
    shot.DrawArc<256>(&shot_arc);
    for (glm::vec3 v3 : shot_arc) {
      line.push_back(v3.x);
      line.push_back(v3.y);
      line.push_back(v3.z);
    }
  }
  LineShader line_shader = CreateLineShader(line.data(), (int)(line.size() / 3));

  std::vector<float> control_points;
  for (int kx=0; kx<problem.backboard_.control_points_.rows(); kx++) {
    for (int ky=0; ky<problem.backboard_.control_points_.cols(); ky++) {
      const glm::dvec3 point = problem.backboard_.control_points_(kx, ky);
      control_points.push_back(static_cast<float>(point.x));
      control_points.push_back(static_cast<float>(point.y));
      control_points.push_back(static_cast<float>(point.z));
    }
  }
  LineShader control_point_shader = CreateLineShader(control_points.data(), (int)(control_points.size() / 3));

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

    // gridmesh
    DrawGridmesh(gridmesh, view, proj);

    // lines
    DrawLines(line_shader, view, proj, glm::vec4(0.1, 0.6, 0.2, 0.025), GL_LINE_STRIP);

    // control points
    DrawLines(control_point_shader, view, proj, glm::vec4(1., 0, 0, 1), GL_POINTS);

    // Swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  FreeGridmeshGlResources(gridmesh);
  FreeCatGlResources(cat_shader);

  glfwDestroyWindow(window);
  glfwTerminate();
  return EXIT_SUCCESS;
}
