#pragma once

#include <glm/glm.hpp>
#include "ray.hpp"

class Primitive {
public:
  virtual glm::vec4 intersect(Ray r);
  virtual ~Primitive();
};

class Sphere : public Primitive {
public:
  virtual glm::vec4 intersect(Ray r);
  virtual ~Sphere();
};

class Cube : public Primitive {
public:
  virtual glm::vec4 intersect(Ray r);
  virtual ~Cube();
};

class NonhierSphere : public Primitive {
public:
  NonhierSphere(const glm::vec3& pos, double radius) : m_pos(pos), m_radius(radius) {}
  virtual ~NonhierSphere();

  virtual glm::vec4 intersect(Ray r);
private:
  const glm::vec3 m_pos;
  const double m_radius;
};

class NonhierBox : public Primitive {
public:
  NonhierBox(const glm::vec3& pos, double size) : m_pos(pos), m_size(size) {}

  virtual glm::vec4 intersect(Ray r);
  virtual ~NonhierBox();

private:
  const glm::vec3 m_pos;
  const double m_size;
};

class IntersectionNotFound: public std::exception {
 private:
  const char* what() const throw() {
    return "Intersection doesn't exist.";
  }
};
