#ifndef AFISKON_OPENGL_CAMERA_H
#define AFISKON_OPENGL_CAMERA_H

#include "linearalg.h"

struct Camera;

Camera* cameraCreate(GLFWwindow* window, const Vector4 startPosition,
	float startHorizontalAngleRad, float startVerticalAngleRad);
void cameraGetPosition(Camera* cam, Vector4* pOutVec);
bool cameraGetMouseInterceptionEnabled(Camera* cam);
void cameraSetMouseInterceptionEnabled(Camera* cam, bool enabled);
void cameraGetViewMatrix(Camera* cam, float deltaTimeMs,
	Matrix* pOutViewMatrix);
void cameraDestroy(Camera* cam);


#endif // AFISKON_OPENGL_CAMERA_H
