#include <algorithm>
#include <iostream>

#include "Primitive.hpp"
#include "polyroots.hpp"

Primitive::~Primitive(){}

glm::vec4 Primitive::intersect(Ray ray) {
  throw IntersectionNotFound();
}

Sphere::~Sphere() {}

glm::vec4 Sphere::intersect(Ray ray) {
  throw IntersectionNotFound();
}

Cube::~Cube() {}

glm::vec4 Cube::intersect(Ray ray) {
  throw IntersectionNotFound();
}

NonhierSphere::~NonhierSphere() {}

glm::vec4 NonhierSphere::intersect(Ray ray) {
  double roots[2];
  const double A = glm::dot(ray.to - ray.from, ray.to - ray.from);
  const double B = 2 * glm::dot(ray.from - glm::vec4(m_pos, 1), ray.to - ray.from);
  const double C = glm::dot(ray.from - glm::vec4(m_pos, 1), ray.from - glm::vec4(m_pos, 1)) - m_radius * m_radius;

  size_t numRoots = quadraticRoots(A, B, C, roots);

  if (numRoots == 0) {
    throw IntersectionNotFound();
  }

  if (numRoots == 1) {
    return ray.from + (ray.to - ray.from) * (float) roots[0];
  }

  return ray.from + (ray.to - ray.from) * (float) std::min(roots[0], roots[1]);
}

NonhierBox::~NonhierBox() {}

glm::vec4 NonhierBox::intersect(Ray ray) {
  throw IntersectionNotFound();
}
