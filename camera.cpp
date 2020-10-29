#include "camera.hpp"

#include <algorithm> // min/max
#include <cmath>

glm::vec3 Camera::Eye() {
  const float elevation = elevation_deg_ * static_cast<float>(M_PI) / 180.f;
  const float azimuth = azimuth_deg_ * static_cast<float>(M_PI) / 180.f;
  const float cos_elev = std::cos(elevation);
  const float x = distance_ * std::cos(azimuth) * cos_elev;
  const float y = distance_ * std::sin(azimuth) * cos_elev;
  const float z = distance_ * std::sin(elevation);
  return Center() + glm::vec3(x, y, z);
}

glm::vec3 Camera::Center() {
  return focus_position_;
}

void Camera::Rotate(const float delta_x, const float delta_y) {
  azimuth_deg_ += delta_x;
  if (azimuth_deg_ > 180) {
    azimuth_deg_ -= 360;
  }
  if (azimuth_deg_ < -180) {
    azimuth_deg_ += 360;
  }

  elevation_deg_ -= delta_y;
  elevation_deg_ = std::max(elevation_deg_, -89.f);
  elevation_deg_ = std::min(elevation_deg_,  89.f);
}

void Camera::Pan(const float delta_x, const float delta_y) {
  const float sin_azi = std::sin(azimuth_deg_ * static_cast<float>(M_PI)/180.f);
  const float cos_azi = std::cos(azimuth_deg_ * static_cast<float>(M_PI)/180.f);
  const float scale = distance_ * 3e-3f;
  focus_position_.y += scale*(delta_x * cos_azi - delta_y * sin_azi);
  focus_position_.x -= scale*(delta_x * sin_azi + delta_y * cos_azi);
}

void Camera::Scroll(const float scroll_amount) {
  if (scroll_amount > 0)  {
    distance_ *= 1.15f;
    distance_ = std::min(distance_, 1e3f);
  } else if (scroll_amount < 0) {
    distance_ /= 1.15f;
    distance_ = std::max(distance_, 1e-3f);
  }
  if (scroll_amount != 1.f && scroll_amount != -1.f && scroll_amount != 0.f) {
    fprintf(stderr, "scroll expected to be in [-1, 0, 1] but it was %.1f\n",
            static_cast<double>(scroll_amount));
  }
}
