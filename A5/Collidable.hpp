#pragma once

#include "Hitbox.hpp"

class Collidable {
public:
  virtual Hitbox getHitbox() = 0;
};
