#pragma once

#include "Collidable.hpp"
#include <glm/glm.hpp>
#include <functional>

class Platform : public Collidable {
  glm::vec3 inertialVelocity;
  glm::vec3 inputVelocity;
  static unsigned int counter;
  double TTL;
  const double initTTL;
public:
  const glm::vec3 size;
  const unsigned int id;
  const double mass;
  Platform(glm::vec3 position, glm::vec3 size, double mass, double ttl);
  glm::vec3 position;
  glm::vec3 acceleration;
  Hitbox getHitbox();

  void decreaseTTL(double t);
  double getTTL() const;
  double getInitTTL() const;
  void resetTTL();

  glm::vec3 getVelocity();
  void setVelocity(glm::vec3);
  void setInputVelocity(glm::vec3);
  void setInertialVelocity(glm::vec3);
};
