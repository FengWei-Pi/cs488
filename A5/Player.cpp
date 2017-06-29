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
