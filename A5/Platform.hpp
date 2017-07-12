#pragma once

#include "Collidable.hpp"
#include <glm/glm.hpp>
#include <functional>

class Platform : public Collidable {
  glm::vec3 inertialVelocity;
  glm::vec3 inputVelocity;
  static unsigned int counter;
public:
  const glm::vec3 size;
  const unsigned int id;
  const double mass;
  double ttl;
  Platform(glm::vec3 position, glm::vec3 size, double mass, double ttl);
  glm::vec3 position;
  glm::vec3 acceleration;
  Hitbox getHitbox();

  glm::vec3 getVelocity();
  void setVelocity(glm::vec3);
  void setInputVelocity(glm::vec3);
  void setInertialVelocity(glm::vec3);
};
