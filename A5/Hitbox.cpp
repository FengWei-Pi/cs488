#include "Hitbox.hpp"

Hitbox::Hitbox(glm::vec3 pos, glm::vec3 size) : position(pos), size(size) {}

Hitbox Hitbox::getIntersection(const Hitbox& other) const {
  glm::vec3 p0 = glm::max(position, other.position);
  glm::vec3 p1 = glm::min(position + size, other.position + other.size);
  return Hitbox{
    p0,
    p1 - p0
  };
}

bool Hitbox::isTrivial() const {
  return size.x <= 0 || size.y <= 0 || size.z <= 0;
}
