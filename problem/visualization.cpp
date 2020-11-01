#include "visualization.hpp"

#include "shader/gridmesh.hpp"
#include "shader/lines.hpp"

ProblemVisualization::ProblemVisualization() : backboard_vis_("image/awesomeface.png") {
  control_points_vis_.point_size_ = 3;
}

void ProblemVisualization::Draw(const glm::mat4 &view, const glm::mat4 &proj) {
  // gridmesh
  backboard_vis_.Draw(view, proj);

  // tangents/normals
  backboard_tangents_vis_.Draw(view, proj, glm::vec4(0.8, 0.8, 0.8, 1.0), GL_LINES);
  backboard_normals_vis_.Draw(view, proj, glm::vec4(0.4, 0.4, 0.6, 1.0), GL_LINES);

  // lines
  shot_lines_vis_.Draw(view, proj, glm::vec4(0.1, 0.6, 0.2, 1.0), GL_LINE_STRIP);
  bounce_lines_vis_.Draw(view, proj, glm::vec4(0.1, 0.2, 0.7, 1.0), GL_LINE_STRIP);

  // control points
  control_points_vis_.Draw(view, proj, glm::vec4(1., 0, 0, 1), GL_POINTS);
}
