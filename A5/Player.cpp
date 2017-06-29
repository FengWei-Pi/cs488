#include "Player.hpp"
#include "Clock.hpp"
#include <algorithm>
#include <cmath>

void Player::setDirection(double dir) {
  oldDirection = getDirection();
  direction = dir;
  t = Clock::getTime();
}

double Player::getDirection() {
  const double deltaT = 0.1;
  const double w = std::min(Clock::getTime() - t, deltaT) / deltaT;

  const double x = (1-w) * std::cos(oldDirection) + w * std::cos(direction);
  const double y = (1-w) * std::sin(oldDirection) + w * std::sin(direction);
  return std::atan2(y, x);
}
