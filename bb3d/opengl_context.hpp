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
    bool KeypressQueueEmpty();
    int PopKeypressQueue();
    std::queue<int> keypress_queue; // to hand to user
    const Camera &GetCamera(); // TODO(greg): delete me
    Camera camera;
    bool IsDraggingOrRotating();
    [[nodiscard]] glm::mat4 GetViewTransformation() const;
    MouseHandler mouse_handler;
  };

  class Window {
  public:
    Window(std::unique_ptr<WindowState> vis);
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
    void PollEvents();
    std::unique_ptr<WindowState> window_state_;
  private:
    GLFWwindow* glfw_window;
  };

}; // namespace bb3d
