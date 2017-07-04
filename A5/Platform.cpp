#include "Platform.hpp"

Platform::Platform(glm::vec3 position, glm::vec3 size) : position(position), size(size) {}

Hitbox Platform::getHitbox() {
  return Hitbox{position, size};
}
