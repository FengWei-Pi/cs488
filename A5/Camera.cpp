#include "Camera.hpp"

Camera::Camera(double yAngle, double xAngle, double zoom)
  : yAngle(yAngle),
    xAngle(xAngle),
    zoom(zoom) {}

glm::mat3 Camera::getXYRotationMatrix() const {
  glm::mat3 rotateY = glm::mat3(glm::rotate(glm::mat4(), float(yAngle), glm::vec3(0, 1, 0)));
  glm::mat3 rotateX = glm::mat3(glm::rotate(glm::mat4(), float(xAngle), glm::vec3(1, 0, 0)));
  return rotateY * rotateX;
}

glm::mat3 Camera::getYRotationMatrix() const {
  glm::mat4 rotateY = glm::rotate(glm::mat4(), float(yAngle), glm::vec3(0, 1, 0));
  return glm::mat3(rotateY);
}
