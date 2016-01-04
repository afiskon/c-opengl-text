#ifndef AFISKON_CAMERA_H
#define AFISKON_CAMERA_H

#include <stdbool.h>
#include "linearalg.h"

struct Camera;
typedef struct Camera Camera;

Camera* cameraCreate(GLFWwindow* window, const Vector startPosition,
	float startHorizontalAngleRad, float startVerticalAngleRad);
void cameraGetPosition(Camera* cam, Vector* pOutVec);
bool cameraGetMouseInterceptionEnabled(Camera* cam);
void cameraSetMouseInterceptionEnabled(Camera* cam, bool enabled);
void cameraGetViewMatrix(Camera* cam, float deltaTimeMs,
	Matrix* pOutViewMatrix);
void cameraDestroy(Camera* cam);


#endif // AFISKON_CAMERA_H
