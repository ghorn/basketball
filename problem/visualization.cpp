#include "visualization.hpp"

#include "shader/gridmesh.hpp"
#include "shader/lines.hpp"

ProblemVisualization::ProblemVisualization() : backboard_vis_("image/awesomeface.png") {
  backboard_tangents_vis_ = CreateLineShader();
  backboard_normals_vis_ = CreateLineShader();
  shot_lines_vis_ = CreateLineShader();
  bounce_lines_vis_ = CreateLineShader();
  control_points_vis_ = CreateLineShader();
  control_points_vis_.point_size = 3;
}

void ProblemVisualization::Draw(const glm::mat4 &view, const glm::mat4 &proj) {
  // gridmesh
  backboard_vis_.Draw(view, proj);

  // tangents/normals
  DrawLines(backboard_tangents_vis_, view, proj, glm::vec4(0.8, 0.8, 0.8, 1.0), GL_LINES);
  DrawLines(backboard_normals_vis_, view, proj, glm::vec4(0.4, 0.4, 0.6, 1.0), GL_LINES);

  // lines
  DrawLines(shot_lines_vis_, view, proj, glm::vec4(0.1, 0.6, 0.2, 1.0), GL_LINE_STRIP);
  DrawLines(bounce_lines_vis_, view, proj, glm::vec4(0.1, 0.2, 0.7, 1.0), GL_LINE_STRIP);

  // control points
  DrawLines(control_points_vis_, view, proj, glm::vec4(1., 0, 0, 1), GL_POINTS);
}
