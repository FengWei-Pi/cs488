#include <algorithm>
#include <iostream>

#include "Primitive.hpp"
#include "polyroots.hpp"
#include "common.hpp"

/**
 * Primitive
 */
Primitive::~Primitive(){}

/**
 * Sphere
 */
Sphere::Sphere() : hitbox(glm::vec3(0, 0, 0), 1) {}

Sphere::~Sphere() {}

double Sphere::intersect(const Ray& ray) {
  return hitbox.intersect(ray);
}

glm::vec4 Sphere::getNormal(const glm::vec4& point) {
  return hitbox.getNormal(point);
}

/**
 * Cube
 */
Cube::Cube() : hitbox(glm::vec3(0, 0, 0), 1) {}

Cube::~Cube() {}

double Cube::intersect(const Ray& ray) {
  return hitbox.intersect(ray);
}

glm::vec4 Cube::getNormal(const glm::vec4& point) {
  return hitbox.getNormal(point);
}

/**
 * NonhierSphere
 */

NonhierSphere::~NonhierSphere() {}

double NonhierSphere::intersect(const Ray& ray) {
  double roots[2];
  const double A = glm::dot(ray.to - ray.from, ray.to - ray.from);
  const double B = 2 * glm::dot(ray.from - glm::vec4(m_pos, 1), ray.to - ray.from);
  const double C = glm::dot(ray.from - glm::vec4(m_pos, 1), ray.from - glm::vec4(m_pos, 1)) - m_radius * m_radius;

  size_t numRoots = quadraticRoots(A, B, C, roots);

  if (
    (numRoots == 0) ||
    (numRoots == 1 && roots[0] < 0) ||
    (numRoots == 2 && roots[0] < 0 && roots[1] < 0)
  ) {
    throw IntersectionNotFound();
  }

  if (numRoots == 1) {
    return roots[0];
  }

  if (roots[0] < 0) {
    return roots[1];
  }

  if (roots[1] < 0) {
    return roots[0];
  }

  return std::min(roots[0], roots[1]);
}

glm::vec4 NonhierSphere::getNormal(const glm::vec4& point) {
  glm::vec3 normal{glm::vec3(point) - m_pos};
  return glm::normalize(glm::vec4(normal, 0));
}

/**
 * NonhierBox
 */

NonhierBox::~NonhierBox() {}

glm::vec4 NonhierBox::getNormal(const glm::vec4& point) {
  double x = 0;
  double y = 0;
  double z = 0;

  double epsilon = 0.001;

  if (std::fabs(point.y - (m_pos.y + m_size)) < epsilon) {
    y += 1;
  }

  if (std::fabs(point.y - m_pos.y) < epsilon) {
    y -= 1;
  }

  if (std::fabs(point.x - (m_pos.x + m_size)) < epsilon) {
    x += 1;
  }

  if (std::fabs(point.x - m_pos.x) < epsilon) {
    x -= 1;
  }

  if (std::fabs(point.z - (m_pos.z + m_size)) < epsilon) {
    z += 1;
  }

  if (std::fabs(point.z - m_pos.z) < epsilon) {
    z -= 1;
  }

  glm::vec4 normal = glm::vec4(glm::normalize(glm::vec3{x, y, z}), 0);

  return normal;
}

double NonhierBox::intersect(const Ray& ray) {
  glm::vec3 x{m_size, 0, 0};
  glm::vec3 y{0, m_size, 0};
  glm::vec3 z{0, 0, m_size};
  glm::vec3 m_pos_end = m_pos + x + y + z;
  glm::vec3 points[18] = {
    m_pos, x, y,
    m_pos, x, z,
    m_pos, y, z,
    m_pos_end, -x, -y,
    m_pos_end, -x, -z,
    m_pos_end, -y, -z
  };

  glm::vec3 A{ray.from};
  glm::vec3 B{ray.to};

  double Infinity = std::numeric_limits<double>::infinity();
  double t = Infinity;

  for (unsigned int i = 0; i < 6; i += 1) {
    glm::vec3 P = points[i * 3];
    glm::vec3 P1mP = points[i * 3 + 1];
    glm::vec3 P2mP = points[i * 3 + 2];

    glm::mat3 M{
      P1mP,
      P2mP,
      A - B
    };

    double determinant = glm::determinant(M);
    if (std::fabs(determinant) < 0.0001) {
      continue;
    }

    glm::vec3 R = A - P;
    glm::vec3 S = glm::inverse(M) * R;

    if (
      !(0.0 <= S.x && S.x <= 1.0) ||
      !(0.0 <= S.y && S.y <= 1.0)
    ) {

      continue;
    }

    if (S.z < t && S.z > 0) {
      t = S.z;
    }
  }

  if (t == Infinity) {
    throw IntersectionNotFound{};
  }

  return t;
}
