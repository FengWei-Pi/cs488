#pragma once

#include <glm/glm.hpp>
#include "ray.hpp"

class Primitive {
public:
  virtual double intersect(const Ray&) = 0;
  virtual glm::vec4 getNormal(const glm::vec4&) = 0;
  virtual ~Primitive();
};


class NonhierSphere : public Primitive {
public:
  NonhierSphere(const glm::vec3& pos, double radius) : m_pos(pos), m_radius(radius) {}
  virtual ~NonhierSphere();

  virtual double intersect(const Ray&);
  virtual glm::vec4 getNormal(const glm::vec4&);
  const glm::vec3 m_pos;
  const double m_radius;
private:
};

class NonhierBox : public Primitive {
public:
  NonhierBox(const glm::vec3& pos, double size) : m_pos(pos), m_size(size) {}
  virtual double intersect(const Ray&);
  virtual glm::vec4 getNormal(const glm::vec4&);
  virtual ~NonhierBox();

private:
  const glm::vec3 m_pos;
  const double m_size;
};

class Sphere : public Primitive {
  NonhierSphere hitbox;
public:
  Sphere();
  virtual double intersect(const Ray&);
  virtual glm::vec4 getNormal(const glm::vec4&);
  virtual ~Sphere();
};

class Cube : public Primitive {
  NonhierBox hitbox;
public:
  Cube();
  virtual double intersect(const Ray&);
  virtual glm::vec4 getNormal(const glm::vec4&);
  virtual ~Cube();
};

class IntersectionNotFound: public std::exception {
 private:
  const char* what() const throw() {
    return "Intersection doesn't exist.";
  }
};


class NormalNotFound: public std::exception {
private:
  const char* what() const throw() {
    return "Normal doesn't exist.";
  }
};
