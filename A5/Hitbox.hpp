#pragma once

#include <glm/glm.hpp>

class Hitbox {
public:
  const glm::vec3 position;
  const glm::vec3 size;
  Hitbox(glm::vec3 pos, glm::vec3 size);
  Hitbox getIntersection(const Hitbox& other) const;
  bool isTrivial() const;
};
