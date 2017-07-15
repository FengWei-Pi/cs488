#include "Platform.hpp"
#include <algorithm>
#include <iostream>

unsigned Platform::counter = 0;

Platform::Platform(glm::vec3 position, glm::vec3 size, double mass, double TTL)
  : position(position),
    id(counter++),
    size(size),
    mass(mass),
    TTL(TTL),
    visited(false),
    initTTL(TTL) {
      assert(TTL > 0);
    }

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

void Platform::decreaseTTL(double dTTL) {
  assert(dTTL > 0);
  TTL = std::max(TTL - dTTL, 0.0);
}

double Platform::getTTL() const{
  return TTL;
}

double Platform::getInitTTL() const{
  return initTTL;
}

void Platform::resetTTL() {
  TTL = initTTL;
}

void Platform::markVisited() {
  visited = true;
}

bool Platform::hasBeenVisited() const {
  return visited;
}
