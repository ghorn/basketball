#include "visualization.hpp"

#include <iostream>

#include "shader/gridmesh.hpp"
#include "shader/lines.hpp"

ProblemVisualization::ProblemVisualization() : backboard_vis_("image/awesomeface.png"), court_vis_("image/warriors_court.png") {
  control_points_vis_.point_size_ = 3;

  Eigen::Matrix<glm::vec3, 2, 2> court = CourtCorners();
  court_vis_.Update(court);
}

void ProblemVisualization::Draw(const glm::mat4 &view, const glm::mat4 &proj) {
  // gridmesh
  backboard_vis_.Draw(view, proj);
  court_vis_.Draw(view, proj);

  // tangents/normals
  backboard_tangents_vis_.Draw(view, proj, glm::vec4(0.8, 0.8, 0.8, 1.0), GL_LINES);
  backboard_normals_vis_.Draw(view, proj, glm::vec4(0.4, 0.4, 0.6, 1.0), GL_LINES);

  // lines
  shot_lines_vis_.Draw(view, proj, glm::vec4(0.1, 0.6, 0.2, 1.0), GL_LINE_STRIP);
  bounce_lines_vis_.Draw(view, proj, glm::vec4(0.1, 0.2, 0.7, 1.0), GL_LINE_STRIP);
  // rim
  rim_vis_.Draw(view, proj, glm::vec4(0.9, 0.1, 0.1, 1.0), GL_LINE_STRIP);


  // control points
  control_points_vis_.Draw(view, proj, glm::vec4(1., 0, 0, 1), GL_POINTS);
}


Eigen::Matrix<glm::vec3, 2, 2> ProblemVisualization::CourtCorners() {
  // Let's use NBA regulations.
  // --------------------------
  constexpr double kFt2M = 0.3048;
  // expected
  constexpr double kExpectedInnerLength = 94 * kFt2M;
  constexpr double kExpectedInnerWidth = 50 * kFt2M;
  // measured
  constexpr double kLengthApronPixels = 0.5*(186 + (1499 - 1318));
  constexpr double kInnerLengthPixels = 1318 - 186;

  constexpr double kWidthApronPixels = 0.5*(83 + (763 - 680));
  constexpr double kInnerWidthPixels = 680 - 83;
  // conversion ratio
  constexpr double kLengthPixelToMeter = kExpectedInnerLength / kInnerLengthPixels;
  constexpr double kWidthPixelToMeter = kExpectedInnerWidth / kInnerWidthPixels;
  constexpr double kPixel2Meter = 0.5*(kLengthPixelToMeter + kWidthPixelToMeter);
  // derive apron size
  constexpr double kLengthApron = kLengthApronPixels * kPixel2Meter; // 3.63*kFt2M;
  constexpr double kWidthApron = kWidthApronPixels * kPixel2Meter; // 8*kFt2M;
  constexpr double kInnerWidth = kInnerWidthPixels * kPixel2Meter;
  constexpr double kInnerLength = kInnerLengthPixels * kPixel2Meter;

  // outer dimensions
  constexpr double kOuterWidth = kInnerWidth + 2 * kWidthApron;
  constexpr double kOuterLength = kInnerLength + 2 * kLengthApron;

  //std::cerr << "kLengthPixelToMeter: " << kLengthPixelToMeter << std::endl;
  //std::cerr << "kWidthPixelToMeter:  " << kWidthPixelToMeter << std::endl;
  //std::cerr << "kPixelToMeter:       " << kPixel2Meter << std::endl;
  //std::cerr << "court outer length: " << kOuterLength/kFt2M << " [ft]" << std::endl;
  //std::cerr << "court outer width:  " << kOuterWidth/kFt2M  << " [ft]" << std::endl;
  //std::cerr << "court inner length: " << kInnerLength/kFt2M << " [ft]" << std::endl;
  //std::cerr << "court inner width:  " << kInnerWidth/kFt2M  << " [ft]" << std::endl;
  //std::cerr << "apron length: " << kLengthApron/kFt2M << std::endl;
  //std::cerr << "apron width:  " << kWidthApron/kFt2M << std::endl;
  //std::cerr << "Aspect ratio: " << kAspectRatio << std::endl;

  Eigen::Matrix<glm::vec3, 2, 2> court;
  court(0, 0) = glm::vec3(-kOuterWidth/2, kOuterLength - kLengthApron, 0);
  court(1, 0) = glm::vec3(-kOuterWidth/2,              - kLengthApron, 0);
  court(0, 1) = glm::vec3( kOuterWidth/2, kOuterLength - kLengthApron, 0);
  court(1, 1) = glm::vec3( kOuterWidth/2,              - kLengthApron, 0);

  return court;
}

