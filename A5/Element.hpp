#pragma once
#include "Visitor.hpp"

class Element {
public:
  virtual void accept(Visitor& v) = 0;
};
