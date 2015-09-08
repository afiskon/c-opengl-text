#include <glm/gtc/matrix_transform.hpp>
#include "camera.h"

Camera::Camera(const glm::vec3& position, float horizontalAngleRad, float verticalAngleRad):
  m_position(position),
  m_horizontalAngleRad(horizontalAngleRad),
  m_verticalAngleRad(verticalAngleRad) { }

void Camera::getViewMatrix(float deltaTimeMs, glm::mat4* pOutViewMatrix) {
  *pOutViewMatrix = glm::lookAt(
    glm::vec3(0, 0, 5),
    glm::vec3(0, 0, 0), // and looks at the origin
    glm::vec3(0, deltaTimeMs > 0.0f? 1 : 1.1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
  );
}