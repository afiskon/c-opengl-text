#ifndef TRIANGLE_CAMERA_H
#define TRIANGLE_CAMERA_H

#include <glm/core/type.hpp>

class Camera {

private:
  GLFWwindow*window;  // TODO: use pImpl
  glm::vec3 position;
  float horizontalAngleRad;
  float verticalAngleRad;

public:
  Camera(GLFWwindow* window, const glm::vec3&startPosition, float startHorizontalAngleRad, float startVerticalAngleRad);
  void getViewMatrix(float deltaTimeMs, glm::mat4* pOutViewMatrix);

};

#endif //TRIANGLE_CAMERA_H
