#ifndef AFISKON_OPENGL_CAMERA_H
#define AFISKON_OPENGL_CAMERA_H

#include <glm/core/type.hpp>

struct Camera;

Camera* cameraCreate(GLFWwindow* window, const glm::vec3&startPosition,
	float startHorizontalAngleRad, float startVerticalAngleRad);
void cameraGetPosition(Camera* cam, glm::vec3* pOutVec);
bool cameraGetMouseInterceptionEnabled(Camera* cam);
void cameraSetMouseInterceptionEnabled(Camera* cam, bool enabled);
void cameraGetViewMatrix(Camera* cam, float deltaTimeMs,
	glm::mat4* pOutViewMatrix);
void cameraDestroy(Camera* cam);


#endif // AFISKON_OPENGL_CAMERA_H
