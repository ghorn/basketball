#pragma once

#include <glm/glm.hpp>

class Camera {
public:
  glm::vec3 Eye();
  glm::vec3 Center();
  void Rotate(const float delta_x, const float delta_y);
  void Pan(const float delta_x, const float delta_y);
  void Scroll(const float scroll_amount);
private:

  glm::vec3 focus_position_;
  float azimuth_deg_ = 60;
  float elevation_deg_ = 30;
  float distance_ = 1;
};
