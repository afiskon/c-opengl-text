#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include "camera.h"

static const float speed = 4.0f; // units per second
static const float mouseSpeedRad = 0.0005f;

struct Camera {
  GLFWwindow* window;
  glm::vec3 position;
  float horizontalAngleRad;
  float verticalAngleRad;
  bool mouseInterceptionEnabled;
};

Camera* cameraCreate(GLFWwindow* window, const glm::vec3& startPosition, float startHorizontalAngleRad, float startVerticalAngleRad) {
  Camera* cam = (Camera*)malloc(sizeof(Camera));
  if(!cam) return nullptr; // malloc failed

  cam->window = window;
  cam->position = startPosition;
  cam->horizontalAngleRad = startHorizontalAngleRad;
  cam->verticalAngleRad = startVerticalAngleRad;
  cam->mouseInterceptionEnabled = true;

  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // hide cursor

  return cam;
}

void cameraDestroy(Camera* cam) {
  free(cam);
}

void cameraGetPosition(Camera* cam, glm::vec3 *pOutVec) {
  *pOutVec = cam->position;
}

bool cameraGetMouseInterceptionEnabled(Camera* cam) {
  return cam->mouseInterceptionEnabled;
}

void cameraSetMouseInterceptionEnabled(Camera* cam, bool enabled) {
  cam->mouseInterceptionEnabled = enabled;
  if(enabled) glfwSetInputMode(cam->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  else glfwSetInputMode(cam->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void cameraGetViewMatrix(Camera* cam, float deltaTimeMs, glm::mat4* pOutViewMatrix) {
  float deltaTimeSec = deltaTimeMs/1000.0f;

  if(cam->mouseInterceptionEnabled) {
    int windowWidth, windowHeight;
    glfwGetWindowSize(cam->window, &windowWidth, &windowHeight);

    double mouseX, mouseY;
    glfwGetCursorPos(cam->window, &mouseX, &mouseY);

    cam->horizontalAngleRad += mouseSpeedRad * (windowWidth/2 - mouseX);
    cam->verticalAngleRad += mouseSpeedRad * (windowHeight/2 - mouseY);

    glfwSetCursorPos(cam->window, windowWidth/2, windowHeight/2);
  }

  glm::vec3 direction(
    cos(cam->verticalAngleRad) * sin(cam->horizontalAngleRad),
    sin(cam->verticalAngleRad),
    cos(cam->verticalAngleRad) * cos(cam->horizontalAngleRad)
  );

  glm::vec3 right = glm::vec3(
    sin(cam->horizontalAngleRad - 3.14f/2.0f),
    0,
    cos(cam->horizontalAngleRad - 3.14f/2.0f)
  );

  glm::vec3 up = glm::cross(right, direction);

  if(glfwGetKey(cam->window, GLFW_KEY_W) == GLFW_PRESS) {
    cam->position += direction * deltaTimeSec * speed;
  }

  if(glfwGetKey(cam->window, GLFW_KEY_S) == GLFW_PRESS) {
    cam->position -= direction * deltaTimeSec * speed;
  }

  if(glfwGetKey(cam->window, GLFW_KEY_A) == GLFW_PRESS) {
    cam->position -= right * deltaTimeSec * speed;
  }

  if(glfwGetKey(cam->window, GLFW_KEY_D) == GLFW_PRESS) {
    cam->position += right * deltaTimeSec * speed;
  }

  *pOutViewMatrix = glm::lookAt(cam->position, cam->position + direction, up);
}