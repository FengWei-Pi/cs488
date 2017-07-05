#pragma once

#include "Collidable.hpp"
#include <glm/glm.hpp>

class Player : public Collidable {
  double direction;
  double oldDirection;
  double t;
  glm::vec3 inertialVelocity;
  glm::vec3 inputVelocity;
  void setDirection(double direction);
public:
  double getDirection();
  bool canWalk = true;
  float mass = 2;
  float speed = 6;
  glm::vec3 position;
  glm::vec3 acceleration;
  Hitbox getHitbox();

  glm::vec3 getVelocity();
  void setInputVelocity(glm::vec3);
  void setVelocity(glm::vec3);
  void clearInertialVelocity();
};
