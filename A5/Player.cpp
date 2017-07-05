#include "Util.hpp"
#include "Player.hpp"
#include "Clock.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>

void Player::setDirection(double dir) {
  oldDirection = getDirection();
  direction = dir;
  t = Clock::getTime();
}

double Player::getDirection() {
  const double deltaT = 0.1;
  const double w = std::min(Clock::getTime() - t, deltaT) / deltaT;
  return Util::mlerp(oldDirection, direction, w, 2 * glm::radians(180.0));
}

Hitbox Player::getHitbox() {
  // Counted from the puppet.lua file
  double height = 8;
  double width = 1;
  double depth = 0.75;

  return Hitbox{
    glm::vec3(position.x - width / 2, position.y, position.z - 0.25),
    glm::vec3(width, height, depth)
  };
}

void Player::clearInertialVelocity() {
  inertialVelocity = glm::vec3(0);
}

glm::vec3 Player::getVelocity() {
  return inertialVelocity + inputVelocity;
}

void Player::setVelocity(glm::vec3 v) {
  inertialVelocity = v - inputVelocity;
}

void Player::setInputVelocity(glm::vec3 inputV) {
  inputVelocity = inputV;
  double inputDir = std::atan2(inputV.x, inputV.z);
  setDirection(inputDir);
}
