#pragma once

#include "Collidable.hpp"
#include <glm/glm.hpp>

class Player : public Collidable {
  double direction;
  double oldDirection;
  double t;
public:
  void setDirection(double direction);
  double getDirection();
  bool isJumping = false;
  glm::vec3 position;
  glm::vec3 velocity;
  glm::vec3 acceleration;
  Hitbox getHitbox();
};
