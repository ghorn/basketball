#include <bits/exception.h>  // for exception
#include <sys/types.h>       // for key_t, uint

#include <algorithm>           // for copy, max
#include <chrono>              // for operator""s, chrono_literals
#include <cstdio>              // for fprintf, stderr
#include <cstdlib>             // for EXIT_SUCCESS
#include <eigen3/Eigen/Dense>  // for Matrix, DenseCoeffsBase
#include <functional>          // for function
#include <iostream>            // for operator<<, basic_ostream, cerr, endl, ostream, cha...
#include <mutex>               // for mutex, lock_guard
#include <optional>            // for optional, nullopt
#include <queue>               // for queue
#include <thread>              // for sleep_for, thread
#include <vector>              // for vector

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>  // for glfwDestroyWindow, glfwGetWindowSize, glfwPollEv...

#include <glm/glm.hpp>  // for mat4
#include <nlopt.hpp>    // for opt, LN_NELDERMEAD

#include "bb3d/assert.hpp"            // for ASSERT
#include "bb3d/opengl_context.hpp"    // for Window
#include "problem/backboard.hpp"      // for Backboard
#include "problem/problem.hpp"        // for Problem
#include "problem/visualization.hpp"  // for ProblemVisualization

constexpr int NX = 6;
constexpr int NY = 4;

constexpr int NU_VIS = 20;
constexpr int NV_VIS = 30;

constexpr int NU_OBJ = 14;
constexpr int NV_OBJ = 8;

Eigen::Matrix<double, NX, NY> Vec2Dvs(const std::vector<double> &vec) {
  ASSERT(NX * NY == vec.size());
  Eigen::Matrix<double, NX, NY> mat;
  int k = 0;
  for (int kx = 0; kx < NX; kx++) {
    for (int ky = 0; ky < NY; ky++) {
      mat(kx, ky) = vec[k];
      k++;
    }
  }
  return mat;
}
std::vector<double> Dvs2Vec(const Eigen::Matrix<double, NX, NY> &mat) {
  std::vector<double> vec;
  vec.reserve(NX * NY);
  for (int kx = 0; kx < NX; kx++) {
    for (int ky = 0; ky < NY; ky++) {
      vec.push_back(mat(kx, ky));
    }
  }
  return vec;
}

struct SharedData {
  std::queue<Eigen::Matrix<double, NX, NY>> dvs_queue;
  std::mutex queue_mutex;
};

double Objective(const std::vector<double> &x, std::vector<double> &grad __attribute__((unused)),
                 void *my_func_data) {
  auto *shared_data = reinterpret_cast<SharedData *>(my_func_data);

  using namespace std::chrono_literals;
  std::this_thread::sleep_for(0.01s);

  Eigen::Matrix<double, NX, NY> dvs = Vec2Dvs(x);

  // First and most importantly, send the design variables to the visualizer.
  {
    const std::lock_guard<std::mutex> lock(shared_data->queue_mutex);
    shared_data->dvs_queue.push(dvs);
  }

  // Now I suppose we could compute the objective.
  return Problem<NX, NY>::ObjectiveFunction<NU_OBJ, NV_OBJ>(
      Backboard<NX, NY>::ToControlPoints(dvs));
}

void Optimize(SharedData &shared_data) {
  std::vector<double> x =
      Dvs2Vec(Backboard<NX, NY>::FromControlPoints(Backboard<NX, NY>::Initialize()));

  nlopt::opt optimizer(nlopt::LN_NELDERMEAD, static_cast<uint>(x.size()));
  // nlopt::opt optimizer(nlopt::LN_SBPLX, static_cast<uint>(x.size()));
  optimizer.set_lower_bounds(-10);
  optimizer.set_upper_bounds(2);

  std::vector<double> dx0(x.size(), 0.1);
  optimizer.set_initial_step(dx0);

  // nlopt_set_xtol_rel(optimizer, 1e-4);
  optimizer.set_xtol_rel(1e-4);

  // FunctionData data = {problem, visualization};
  optimizer.set_min_objective(Objective, &shared_data);

  //  opt.add_inequality_constraint(myvconstraint, &data[0], 1e-8);
  //  opt.add_inequality_constraint(myvconstraint, &data[1], 1e-8);
  //  std::vector<double> x(2);
  //  x[0] = 1.234; x[1] = 5.678;

  double minf{};
  try {
    fprintf(stderr, "starting optimization\n");
    optimizer.optimize(x, minf);
    fprintf(stderr, "found minimum %.12f\n", minf);
    // return EXIT_SUCCESS;
  } catch (std::exception &e) {
    std::cerr << "nlopt failed: " << e.what() << std::endl;
    // return EXIT_FAILURE;
  }
}

int run_it() {
  // Boilerplate
  bb3d::Window window;

  // problem
  ProblemVisualization visualization;
  visualization.Update<NU_OBJ, NV_OBJ, NU_VIS, NU_VIS>(Backboard<NX, NY>::Initialize());

  // it's theadn' time
  SharedData shared_data;
  std::thread thread_object([&shared_data]() { Optimize(shared_data); });

  std::function<void(key_t)> handle_keypress = [&visualization](key_t key) {
    visualization.HandleKeyPress(key);
  };

  std::function<void()> update_visualization = [&visualization, &shared_data]() {
    // drain the queue
    std::optional<Eigen::Matrix<double, NX, NY>> dvs = std::nullopt;
    {
      const std::lock_guard<std::mutex> lock(shared_data.queue_mutex);
      while (shared_data.dvs_queue.size() > 1) {
        shared_data.dvs_queue.pop();
      }
      if (!shared_data.dvs_queue.empty()) {
        dvs = shared_data.dvs_queue.front();
        shared_data.dvs_queue.pop();
      }
    }
    if (dvs) {
      visualization.Update<NU_OBJ, NV_OBJ, NU_VIS, NU_VIS>(
          Backboard<NX, NY>::ToControlPoints(*dvs));
    }
  };

  std::function<void(const glm::mat4 &, const glm::mat4 &)> draw_visualization =
      [&visualization](const glm::mat4 &view, const glm::mat4 &proj) {
        visualization.Draw(view, proj);
      };

  window.Run(handle_keypress, update_visualization, draw_visualization);

  return EXIT_SUCCESS;
}

int main(int argc __attribute__((unused)), char *argv[] __attribute__((unused))) {
  try {
    run_it();
  } catch (const std::exception &e) {
    std::cerr << e.what();
  }
}
