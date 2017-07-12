#include "Platform.hpp"
#include <iostream>

unsigned Platform::counter = 0;

Platform::Platform(glm::vec3 position, glm::vec3 size)
  : position(position),
    id(counter++),
    size(size) {}

Hitbox Platform::getHitbox() {
  return Hitbox{position, size};
}

glm::vec3 Platform::getVelocity() {
  return inertialVelocity + inputVelocity;
}

void Platform::setVelocity(glm::vec3 v) {
  inertialVelocity = v - inputVelocity;
}

void Platform::setInputVelocity(glm::vec3 inputV) {
  inputVelocity = inputV;
}

void Platform::setInertialVelocity(glm::vec3 v) {
  inertialVelocity = v;
}
