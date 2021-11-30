#pragma once

#include <glm/glm.hpp>

class Camera {
public:
  [[nodiscard]] glm::vec3 Eye() const;
  [[nodiscard]] glm::vec3 Center() const;
  [[nodiscard]] double Distance() const;
  void Rotate(const float delta_x, const float delta_y);
  void TranslateXy(const float delta_x, const float delta_y);
  void TranslateZ(const float delta_x, const float delta_y);
  void Scroll(const float scroll_amount);
private:
  glm::vec3 focus_position_ = {0, 0, -2};
  float azimuth_deg_ = 60;
  float elevation_deg_ = 30;
  float distance_ = 10;
};
