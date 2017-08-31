#include "Util.hpp"
#include "Player.hpp"
#include "Clock.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>

Player::Player(glm::vec3 Fg)
  : mass(1),
    runningSpeed(6),
    jumpingSpeed(10),
    g(Fg/mass),
    position(0, 4, 0),
    power(0.0),
    oldDirection(0),
    direction(0),
    t(Clock::getTime()),
    inertialVelocity(0),
    inputVelocity(0)
{}

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

glm::vec3 Player::getVelocity() {
  return inertialVelocity + inputVelocity;
}

void Player::setVelocity(glm::vec3 v) {
  inertialVelocity = v - inputVelocity;
}

void Player::setInputVelocity(glm::vec3 inputV) {
  inputVelocity = inputV;
  setDirection(inputV);
}

void Player::setDirection(glm::vec3 inputV) {
  const double epsilon = 0.0001;

  if (glm::length(glm::vec2(inputV.x, inputV.z)) >= epsilon) {
    double inputDir = std::atan2(inputV.x, inputV.z);
    setDirection(inputDir);
  }
}

void Player::setInertialVelocity(glm::vec3 v) {
  inertialVelocity = v;
}
