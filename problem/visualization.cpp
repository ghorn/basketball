#include "visualization.hpp"

#include "shader/gridmesh.hpp"
#include "shader/lines.hpp"

ProblemVisualization::ProblemVisualization() : backboard_vis_("image/awesomeface.png"), court_vis_("image/court.png") {
  control_points_vis_.point_size_ = 3;
  // Let's use NBA regulations.
  constexpr double kFt2M = 0.3048;
  constexpr double kCourtInnerLength = 94 * kFt2M;
  constexpr double kCourtInnerWidth = 50 * kFt2M;
  constexpr double kWidthApron = 5*kFt2M;
  constexpr double kLengthApron = 8*kFt2M;
  constexpr double kCourtOuterWidth = kCourtInnerWidth + kWidthApron;
  Eigen::Matrix<glm::vec3, 2, 2> court;
  court(0, 0) = glm::vec3(-kCourtOuterWidth/2,                   - kLengthApron, 0);
  court(1, 0) = glm::vec3( kCourtOuterWidth/2,                   - kLengthApron, 0);
  court(0, 1) = glm::vec3(-kCourtOuterWidth/2, kCourtInnerLength + kLengthApron, 0);
  court(1, 1) = glm::vec3( kCourtOuterWidth/2, kCourtInnerLength + kLengthApron, 0);

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

  // control points
  control_points_vis_.Draw(view, proj, glm::vec4(1., 0, 0, 1), GL_POINTS);
}
