#pragma once

#include <glm/glm.hpp>

class Player {
  double direction;
  double oldDirection;
  double t;
public:
  void setDirection(double direction);
  double getDirection();
  glm::vec3 position;
  glm::vec3 velocity;
  glm::vec3 acceleration;
};
