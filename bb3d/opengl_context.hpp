#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <memory>
#include <queue>                         // for queue

#include "bb3d/camera.hpp"

namespace bb3d {

  struct MouseHandler {
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

  // The state which is stored by glfwSetWindowUserPointer.
  class WindowState {
  public:
    WindowState()= default;;
    ~WindowState()= default;;
    [[nodiscard]] bool KeypressQueueEmpty() const;
    int PopKeypressQueue();
    std::queue<int> keypress_queue; // to hand to user
    [[nodiscard]] const Camera &GetCamera() const; // TODO(greg): delete me
    Camera camera;
    [[nodiscard]] bool IsDraggingOrRotating() const;
    [[nodiscard]] glm::mat4 GetViewTransformation() const;
    MouseHandler mouse_handler{};
  };

  class Window {
  public:
    explicit Window(std::unique_ptr<WindowState> window_state);
    ~Window();
    struct Size {
      int width;
      int height;
    };

    bool ShouldClose();
    [[nodiscard]] Size GetSize() const;
    [[nodiscard]] glm::mat4 GetProjectionTransformation() const;
    [[nodiscard]] glm::mat4 GetOrthographicProjection() const;
    void SwapBuffers();
    static void PollEvents();
    std::unique_ptr<WindowState> &GetWindowState() {return window_state_;};
  private:
    GLFWwindow* glfw_window;
    std::unique_ptr<WindowState> window_state_;
  };

}; // namespace bb3d
