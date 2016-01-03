#include <glm/gtc/matrix_transform.hpp> // TODO: delete

#include <GLFW/glfw3.h>
#include "camera.h"
#include "linearalg.h"

static const float CAMERA_SPEED = 4.0f; // units per second
static const float MOUSE_SPEED_RAD = 0.0005f;

struct Camera
{
    GLFWwindow* window;
    Vector4 position;
    float horizontalAngleRad;
    float verticalAngleRad;
    bool mouseInterceptionEnabled;
};

Camera*
cameraCreate(GLFWwindow* window, const Vector4 startPosition,
    float startHorizontalAngleRad, float startVerticalAngleRad)
{
    Camera* cam = (Camera*)malloc(sizeof(Camera));
    if(cam == NULL)
        return NULL; // malloc failed

    cam->window = window;
    cam->position = startPosition;
    cam->horizontalAngleRad = startHorizontalAngleRad;
    cam->verticalAngleRad = startVerticalAngleRad;
    cam->mouseInterceptionEnabled = true;

    // hide cursor
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    return cam;
}

void
cameraDestroy(Camera* cam)
{
    free(cam);
}

void
cameraGetPosition(Camera* cam, Vector4* pOutVec)
{
    *pOutVec = cam->position;
}

bool
cameraGetMouseInterceptionEnabled(Camera* cam)
{
    return cam->mouseInterceptionEnabled;
}

void
cameraSetMouseInterceptionEnabled(Camera* cam, bool enabled)
{
    cam->mouseInterceptionEnabled = enabled;
    if(enabled)
        glfwSetInputMode(cam->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    else
        glfwSetInputMode(cam->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void
cameraGetViewMatrix(Camera* cam, float deltaTimeMs, Matrix* pOutViewMatrix)
{
    float deltaTimeSec = deltaTimeMs/1000.0f;

    if(cam->mouseInterceptionEnabled)
    {
        int windowWidth, windowHeight;
        glfwGetWindowSize(cam->window, &windowWidth, &windowHeight);

        double mouseX, mouseY;
        glfwGetCursorPos(cam->window, &mouseX, &mouseY);

        cam->horizontalAngleRad += MOUSE_SPEED_RAD * (windowWidth/2 - mouseX);
        cam->verticalAngleRad += MOUSE_SPEED_RAD * (windowHeight/2 - mouseY);

        glfwSetCursorPos(cam->window, windowWidth/2, windowHeight/2);
    }

    Vector4 direction = {{
            (float)(cos(cam->verticalAngleRad) * sin(cam->horizontalAngleRad)),
            (float)(sin(cam->verticalAngleRad)),
            (float)(cos(cam->verticalAngleRad) * cos(cam->horizontalAngleRad)),
            0.0f
        }};

    Vector4 right = {{
            (float)(sin(cam->horizontalAngleRad - 3.14f/2.0f)),
            0.0f,
            (float)(cos(cam->horizontalAngleRad - 3.14f/2.0f)),
            0.0f
        }};

    Vector4 up = crossvec4(right, direction);
    float delta = deltaTimeSec * CAMERA_SPEED;

    if(glfwGetKey(cam->window, GLFW_KEY_W) == GLFW_PRESS)
        cam->position = addvec4(cam->position, mulvec4(direction, delta));

    if(glfwGetKey(cam->window, GLFW_KEY_S) == GLFW_PRESS)
        cam->position = addvec4(cam->position, mulvec4(direction, -delta));

    if(glfwGetKey(cam->window, GLFW_KEY_A) == GLFW_PRESS)
        cam->position = addvec4(cam->position, mulvec4(right, -delta));

    if(glfwGetKey(cam->window, GLFW_KEY_D) == GLFW_PRESS)
        cam->position = addvec4(cam->position, mulvec4(right, delta));

    // *pOutViewMatrix = glm::lookAt(cam->position,
    //                               cam->position + direction,
    //                               up
    //                             );

    *pOutViewMatrix = lookAt(
                            cam->position,
                            addvec4(cam->position, direction),
                            up
                        );

    glm::vec3 posTemp = glm::vec3(cam->position.x, cam->position.y, cam->position.z);
    glm::vec3 dirTemp = glm::vec3(direction.x, direction.y, direction.z);
    glm::vec3 upTemp = glm::vec3(up.x, up.y, up.z);
    glm::mat4 viewExpected = glm::lookAt(posTemp,
                              posTemp + dirTemp,
                              upTemp
                            );

    for(int ti = 0; ti < 4; ti++)
        for(int tj = 0; tj < 4; tj++)
            pOutViewMatrix->m[ti*4 + tj] = viewExpected[ti][tj];
}