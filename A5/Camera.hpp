#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
  double yAngle;
  double xAngle;
  double zoom;

  Camera(double yAngle, double xAngle, double zoom);

  glm::mat3 getXYRotationMatrix() const;
  glm::mat3 getYRotationMatrix() const;
};
