#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include "camera.h"

// TODO: rewrite in pure C!
// cameraCreate
// cameraGetPosition
// cameraGetMouseInterception
// cameraSetMouseInterception
// cameraGetViewMatrix

static const float speed = 4.0f; // units per second
static const float mouseSpeedRad = 0.0005f;

Camera::Camera(GLFWwindow* window, const glm::vec3& startPosition, float startHorizontalAngleRad, float startVerticalAngleRad):
  window(window),
  position(startPosition),
  horizontalAngleRad(startHorizontalAngleRad),
  verticalAngleRad(startVerticalAngleRad),
  mouseInterceptionEnabled(true) {

  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // hide cursor
}

void Camera::getPosition(glm::vec3 *pOutVec) {
  *pOutVec = position;
}

bool Camera::getMouseInterception() {
  return mouseInterceptionEnabled;
}

void Camera::setMouseInterception(bool enabled) {
  mouseInterceptionEnabled = enabled;
  if(enabled) glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  else glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void Camera::getViewMatrix(float deltaTimeMs, glm::mat4* pOutViewMatrix) {
  float deltaTimeSec = deltaTimeMs/1000.0f;

  if(mouseInterceptionEnabled) {
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);

    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    horizontalAngleRad += mouseSpeedRad * (windowWidth/2 - mouseX);
    verticalAngleRad += mouseSpeedRad * (windowHeight/2 - mouseY);

    glfwSetCursorPos(window, windowWidth/2, windowHeight/2);
  }

  glm::vec3 direction(
    cos(verticalAngleRad) * sin(horizontalAngleRad),
    sin(verticalAngleRad),
    cos(verticalAngleRad) * cos(horizontalAngleRad)
  );

  glm::vec3 right = glm::vec3(
    sin(horizontalAngleRad - 3.14f/2.0f),
    0,
    cos(horizontalAngleRad - 3.14f/2.0f)
  );

  glm::vec3 up = glm::cross(right, direction);

  if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    position += direction * deltaTimeSec * speed;
  }

  if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    position -= direction * deltaTimeSec * speed;
  }

  if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    position -= right * deltaTimeSec * speed;
  }

  if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    position += right * deltaTimeSec * speed;
  }

  *pOutViewMatrix = glm::lookAt(position, position + direction, up);
}