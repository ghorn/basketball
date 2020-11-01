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
  GridmeshShader gridmesh = CreateGridmeshFromMatrix(problem.backboard_.Interpolate<20, 30>().position);

  // Line rendering
  std::vector<std::vector<glm::vec3> > shot_lines;
  for (const Shot &shot : shots) {
    std::array<glm::vec3, 256> shot_arc;
    shot.DrawArc<256>(&shot_arc);

    std::vector<glm::vec3> segment;
    for (glm::vec3 v3 : shot_arc) {
      segment.push_back(v3);
    }
    shot_lines.push_back(segment);
  }

  LineShader line_shader = CreateLineShader();
  UpdateLines(line_shader, shot_lines);

  std::vector<glm::vec3> control_points;
  for (int kx=0; kx<problem.backboard_.control_points_.rows(); kx++) {
    for (int ky=0; ky<problem.backboard_.control_points_.cols(); ky++) {
      const glm::dvec3 point = problem.backboard_.control_points_(kx, ky);
      control_points.push_back(point);
    }
  }
  LineShader control_point_shader = CreateLineShader();
  std::vector<std::vector<glm::vec3> > all_control_points;
  all_control_points.push_back(control_points);
  UpdateLines(control_point_shader, all_control_points);

  // Cat
  CatShader cat_shader = CreateCatShader(image_path);

  std::chrono::time_point t_start = std::chrono::high_resolution_clock::now();

  int counter = 0;
  while (glfwWindowShouldClose(window) == false) {
    counter++;
    if (counter == 100) {
      fprintf(stderr, "UPDATING LINES HOLLA\n");
      for (std::vector<glm::vec3> &segment : shot_lines) {
        for (glm::vec3 &vertex : segment) {
          vertex -= glm::vec3(0.f, 0.f, -0.5f);
        }
      }
      UpdateLines(line_shader, shot_lines);
    }

    if (counter == 200) {
      fprintf(stderr, "UPDATING LINES HOLLA\n");
      shot_lines.resize(shot_lines.size()/3);
      UpdateLines(line_shader, shot_lines);
    }

    if (counter == 300) {
      fprintf(stderr, "UPDATING LINES HOLLA\n");
      for (std::vector<glm::vec3> &segment : shot_lines) {
        segment.resize(segment.size()/3);
      }
      UpdateLines(line_shader, shot_lines);
    }

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
    DrawLines(line_shader, view, proj, glm::vec4(0.1, 0.6, 0.2, 1.0), GL_LINE_STRIP);

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
