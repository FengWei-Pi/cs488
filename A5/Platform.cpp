#include "Platform.hpp"

Platform::Platform(glm::vec3 position, glm::vec3 size, std::function<glm::vec3(glm::vec3)> fn)
  : position(position),
    size(size),
    updateV(fn) {}

Hitbox Platform::getHitbox() {
  return Hitbox{position, size};
}

glm::vec3 Platform::getVelocity() {
  velocity = updateV(velocity);
  return velocity;
}
