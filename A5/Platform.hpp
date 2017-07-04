#pragma once

#include "Collidable.hpp"
#include <glm/glm.hpp>

class Platform : public Collidable {
public:
  const glm::vec3 position;
  const glm::vec3 size;
  Platform(glm::vec3 position, glm::vec3 size);
  Hitbox getHitbox();
};
