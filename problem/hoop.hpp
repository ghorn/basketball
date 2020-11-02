#pragma once

class Hoop {
public:
  static constexpr double kRimHeight = 3.05; // 10 feet
  static constexpr double kRimDiameter = 0.4572; // 18"
  static constexpr double kRimBackboardOffset = 0.151; // 6"
  static constexpr double kRimCenterY = kRimBackboardOffset + kRimDiameter;
  static glm::dvec3 RimCenter() {
    return {0, kRimCenterY, -kRimHeight};
  }

  static std::vector<glm::vec3> DrawArc() {
    constexpr int N = 24;
    constexpr double kRimRadius = kRimDiameter / 2;
    std::vector<glm::vec3> ret;
    ret.reserve(N);
    for (int k=0; k < N; k++) {
      const double theta = 2 * M_PI * k / (double)(N - 1);
      ret.push_back({kRimRadius*sin(theta), kRimCenterY + kRimRadius*cos(theta), -kRimHeight});
    }
    return ret;
  }
};
