#include "visualization.hpp"

#include <GL/glew.h>            // for glPolygonMode, GL_LINE_STRIP, GL_FRONT_AND_BACK, GL_LINES
#include <iostream>             // for operator<<, cerr, ostream, char_traits, endl, basic_ostream
#include <string>               // for allocator, operator<<, string

#include "bb3d/shader/cubemesh.hpp"  // for Cubemesh
#include "bb3d/shader/gridmesh.hpp"  // for Gridmesh
#include "bb3d/shader/lines.hpp"     // for Lines

ProblemVisualization::ProblemVisualization() : backboard_vis_("image/awesomeface.png"), court_vis_("image/warriors_court.png") {
  control_points_vis_.SetPointSize(3);

  Eigen::Matrix<glm::vec3, 2, 2> court = CourtCorners();
  court_vis_.Update(court);
}

void ProblemVisualization::Draw(const glm::mat4 &view, const glm::mat4 &proj) {
  if (wireframe_on_) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  } else {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }

  // backboard
  backboard_vis_.Draw(view, proj);

  // court
  if (court_on_) {
    court_vis_.Draw(view, proj);
  }

  // tangents/normals
  if (tangents_on_) {
    backboard_tangents_vis_.Draw(view, proj, glm::vec4(0.8, 0.8, 0.8, 1.0), GL_LINES);
  }
  if (normals_on_) {
    backboard_normals_vis_.Draw(view, proj, glm::vec4(0.4, 0.4, 0.6, 1.0), GL_LINES);
  }

  // rim
  rim_vis_.Draw(view, proj, glm::vec4(0.9, 0.1, 0.1, 1.0), GL_LINE_STRIP);

  // shot
  if (shots_on_) {
    shot_lines_vis_.Draw(view, proj, GL_LINE_STRIP);
  }
  if (bounces_on_) {
    bounce_lines_vis_.Draw(view, proj, GL_LINE_STRIP);
  }

  // control points
  if (control_points_on_) {
    control_points_vis_.Draw(view, proj, glm::vec4(1., 0, 0, 1), GL_POINTS);
  }

  // histogram
  if (histogram_on_) {
    histogram_vis_.Draw(view, proj);
  }
}

static void DescribeState(const std::string& name, bool state) {
  std::cerr << name << " drawing ";
  if (state) {
    std::cerr << "enabled";
  } else {
    std::cerr << "disabled";
  }
  std::cerr << std::endl;
}

void ProblemVisualization::HandleKeyPress(const int key) {
  switch (key) {
  case GLFW_KEY_S: {
    shots_on_ = !shots_on_;
    DescribeState("shot", shots_on_);
    break;
  }
  case GLFW_KEY_B: {
    bounces_on_ = !bounces_on_;
    DescribeState("bounce", bounces_on_);
    break;
  }
  case GLFW_KEY_C: {
    court_on_ = !court_on_;
    DescribeState("court", court_on_);
    break;
  }
  case GLFW_KEY_H: {
    histogram_on_ = !histogram_on_;
    DescribeState("histogram", histogram_on_);
    break;
  }
  case GLFW_KEY_P: {
    control_points_on_ = !control_points_on_;
    DescribeState("control points", control_points_on_);
    break;
  }
  case GLFW_KEY_N: {
    normals_on_ = !normals_on_;
    DescribeState("normal", normals_on_);
    break;
  }
  case GLFW_KEY_T: {
    tangents_on_ = !tangents_on_;
    DescribeState("tangent", tangents_on_);
    break;
  }
  case GLFW_KEY_W: {
    wireframe_on_ = !wireframe_on_;
    DescribeState("wireframe", wireframe_on_);
    break;
  }
  default:{
  }
  }
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

