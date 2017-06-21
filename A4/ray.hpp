#pragma once

#include <glm/glm.hpp>
#include <exception>

struct Ray {
  const glm::vec4 from;
  const glm::vec4 to;
  glm::vec4 at(double t) const {
    return from + (to - from) * ((float) t);
  };
  Ray(const glm::vec4& from, const glm::vec4& to) : from(from), to(to) {}
};
