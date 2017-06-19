#pragma once

#include <glm/glm.hpp>
#include <exception>

struct Ray {
  const glm::vec4 from;
  const glm::vec4 to;
  Ray(const glm::vec4& from, const glm::vec4& to) : from(from), to(to) {}
};
