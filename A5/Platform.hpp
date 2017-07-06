#pragma once

#include "Collidable.hpp"
#include <glm/glm.hpp>
#include <functional>

class Platform : public Collidable {
  glm::vec3 velocity;
  const std::function<glm::vec3(glm::vec3)> updateV;
public:
  glm::vec3 getVelocity();
  glm::vec3 position;
  const glm::vec3 size;
  Platform(glm::vec3 position, glm::vec3 size, std::function<glm::vec3(glm::vec3)> fn);
  Hitbox getHitbox();
};
