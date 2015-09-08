#ifndef TRIANGLE_CAMERA_H
#define TRIANGLE_CAMERA_H

#include <glm/core/type.hpp>

class Camera {
private:
  glm::vec3 m_position; // TODO: use pImpl
  float m_horizontalAngleRad;
  float m_verticalAngleRad;

public:
  Camera(const glm::vec3& position, float horizontalAngleRad, float verticalAngleRad);
  void getViewMatrix(float deltaTimeMs, glm::mat4* pOutViewMatrix);

};

#endif //TRIANGLE_CAMERA_H
