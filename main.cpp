#include <GL/glew.h>                     // for glClear, glClearColor, GL_COLOR_BUFFER_BIT, GL_D...
#include <bits/exception.h>              // for exception
#include <chrono>                        // for duration, duration_cast, operator-, high_resolut...
#include <cstdio>                        // for fprintf, sprintf, stderr
#include <cstdlib>                       // for EXIT_SUCCESS
#include <eigen3/Eigen/Dense>            // for Matrix, DenseCoeffsBase
#include <iostream>                      // for operator<<, basic_ostream, endl, cerr, ostream
#include <mutex>                         // for mutex, lock_guard
#include <optional>                      // for optional, nullopt
#include <queue>                         // for queue
#include <string>                        // for allocator, char_traits, string
#include <sys/types.h>                   // for uint
#include <thread>                        // for sleep_for, thread
#include <vector>                        // for vector
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>                  // for glfwDestroyWindow, glfwGetWindowSize, glfwPollEv...
#include <glm/glm.hpp>                   // for operator+, vec3, mat4, radians, vec4
#include <glm/gtc/matrix_transform.hpp>  // for rotate
#include <nlopt.hpp>                     // for opt, LN_NELDERMEAD

#include "bb3d/assert.hpp"                    // for ASSERT
#include "bb3d/camera.hpp"                    // for Camera
#include "bb3d/opengl_context.hpp"            // for GetCamera, GetProjectionTransformation, GetViewT...
#include "bb3d/shader/colorlines.hpp"         // for ColoredVec3, ColorLines
#include "bb3d/shader/freetype.hpp"           // for Freetype
#include "problem/backboard.hpp"         // for Backboard
#include "problem/problem.hpp"           // for Problem
#include "problem/visualization.hpp"     // for ProblemVisualization

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

constexpr int NX = 6;
constexpr int NY = 4;

constexpr int NU_VIS = 20;
constexpr int NV_VIS = 30;

constexpr int NU_OBJ = 14;
constexpr int NV_OBJ = 8;

Eigen::Matrix<double, NX, NY> Vec2Dvs(const std::vector<double> &vec) {
  ASSERT(NX*NY == vec.size());
  Eigen::Matrix<double, NX, NY> mat;
  int k = 0;
  for (int kx=0; kx<NX; kx++) {
    for (int ky=0; ky<NY; ky++) {
      mat(kx, ky) = vec[k];
      k++;
    }
  }
  return mat;
}
std::vector<double> Dvs2Vec(const Eigen::Matrix<double, NX, NY> &mat) {
  std::vector<double> vec;
  vec.reserve(NX*NY);
  for (int kx=0; kx<NX; kx++) {
    for (int ky=0; ky<NY; ky++) {
      vec.push_back(mat(kx, ky));
    }
  }
  return vec;
}

std::queue<Eigen::Matrix<double, NX, NY> > dvs_queue;
std::mutex g_queue_mutex;

double Objective(const std::vector<double> &x,
                 std::vector<double> &grad __attribute__((unused)),
                 void *my_func_data __attribute__((unused))) {
  using namespace std::chrono_literals;
  std::this_thread::sleep_for(0.01s);

  Eigen::Matrix<double, NX, NY> dvs = Vec2Dvs(x);

  // First and most importantly, send the design variables to the visualizer.
  {
    const std::lock_guard<std::mutex> lock(g_queue_mutex);
    dvs_queue.push(dvs);
  }

  // Now I suppose we could compute the objective.
  return Problem<NX, NY>::ObjectiveFunction<NU_OBJ, NV_OBJ>(Backboard<NX, NY>::ToControlPoints(dvs));
}

void Optimize() {
  std::vector<double> x =
    Dvs2Vec(Backboard<NX, NY>::FromControlPoints(Backboard<NX, NY>::Initialize()));

  nlopt::opt optimizer(nlopt::LN_NELDERMEAD, static_cast<uint>(x.size()));
  //nlopt::opt optimizer(nlopt::LN_SBPLX, static_cast<uint>(x.size()));
  optimizer.set_lower_bounds(-10);
  optimizer.set_upper_bounds(2);

  std::vector<double> dx0(x.size(), 0.1);
  optimizer.set_initial_step(dx0);


  //nlopt_set_xtol_rel(optimizer, 1e-4);
  optimizer.set_xtol_rel(1e-4);

  //FunctionData data = {problem, visualization};
  optimizer.set_min_objective(Objective, nullptr);

//  opt.add_inequality_constraint(myvconstraint, &data[0], 1e-8);
//  opt.add_inequality_constraint(myvconstraint, &data[1], 1e-8);
//  std::vector<double> x(2);
//  x[0] = 1.234; x[1] = 5.678;

  double minf;
  try{
    fprintf(stderr, "starting optimization\n");
    optimizer.optimize(x, minf);
    fprintf(stderr, "found minimum %.12f\n", minf);
    //return EXIT_SUCCESS;
  } catch (std::exception &e) {
    std::cerr << "nlopt failed: " << e.what() << std::endl;
    //return EXIT_FAILURE;
  }
}

int main(int argc __attribute__((unused)),
         char * argv[] __attribute__((unused))) {
  // Boilerplate
  GLFWwindow * const window = OpenglSetup();

  // problem
  ProblemVisualization visualization;
  visualization.Update<NU_OBJ, NV_OBJ, NU_VIS, NU_VIS>(Backboard<NX, NY>::Initialize());

  // it's theadn' time
  std::thread thread_object([]() {Optimize();});

  ColorLines axes;

  Freetype textbox(18);

  const std::chrono::time_point t_start = std::chrono::high_resolution_clock::now();
  std::chrono::time_point t_last = t_start;
  while (glfwWindowShouldClose(window) == false) {
    // Send keypress events to visualization to update state.
    while (!KeypressQueueEmpty()) {
      visualization.HandleKeyPress(PopKeypressQueue());
    }

    // drain the queue
    std::optional<Eigen::Matrix<double, NX, NY> > dvs = std::nullopt;
    {
      const std::lock_guard<std::mutex> lock(g_queue_mutex);
      while (dvs_queue.size() > 1) {
        dvs_queue.pop();
      }
      if (dvs_queue.size() > 0) {
        dvs = dvs_queue.front();
        dvs_queue.pop();
      }
    }
    if (dvs) {
      visualization.Update<NU_OBJ, NV_OBJ, NU_VIS, NU_VIS>(Backboard<NX, NY>::ToControlPoints(*dvs));
    }

    std::chrono::time_point t_now = std::chrono::high_resolution_clock::now();
    float frame_time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_last).count();
    t_last = t_now;

    // Clear the screen to black
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Calculate transformation
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

    // Draw some dummy text.
    char fps_string[80];
    sprintf(fps_string, "%.1f fps", 1 / frame_time);
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    textbox.RenderText(window, std::string(fps_string), 25.0f, (float)height-25.0f, glm::vec3(1, 1, 1));

    // Swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  return EXIT_SUCCESS;
}
