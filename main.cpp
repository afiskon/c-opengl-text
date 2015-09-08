#include <GLXW/glxw.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <defer.h>
#include <chrono>
#include "utils/utils.h"

static const GLfloat globVertexBufferData[] = {
  -1.0f, -1.0f,  0.0f,
   1.0f, -1.0f,  0.0f,
   0.0f,  1.0f,  0.0f,
};

void windowSizeCallback(GLFWwindow *, int width, int height) {
  glViewport(0, 0, width, height);
}

// TODO: box instead of triangle
// TODO: mouse, keyboard (camera.cpp / hpp ? )

int main() {
  if(glfwInit() == GL_FALSE) {
    std::cerr << "Failed to initialize GLFW" << std::endl;
    return -1;
  }
  defer(std::cout << "Calling glfwTerminate()" << std::endl; glfwTerminate());

  glfwDefaultWindowHints();

  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

  GLFWwindow* window = glfwCreateWindow(800, 600, "Triangle", nullptr, nullptr);
  if(window == nullptr) {
    std::cerr << "Failed to open GLFW window" << std::endl;
    return -1;
  }
  defer(std::cout << "Calling glfwDestroyWindow()" << std::endl; glfwDestroyWindow(window));

  glfwMakeContextCurrent(window);

  if(glxwInit()) {
    std::cerr << "Failed to init GLXW" << std::endl;
    return -1;
  }

  glfwSwapInterval(1);

  glfwSetWindowSizeCallback(window, windowSizeCallback);

  glfwShowWindow(window);

  glEnable(GL_DEPTH_TEST | GL_DOUBLEBUFFER | GL_CULL_FACE); // TODO: describe GL_CULL_FACE!
  glDepthFunc(GL_LESS);

  glClearColor(0, 0, 0, 1);

  bool errorFlag = false;
  std::vector<GLuint> shaders;

  GLuint vertexShaderId = loadShader("shaders/vertexShader.glsl", GL_VERTEX_SHADER, &errorFlag);
  if(errorFlag) {
    std::cerr << "Failed to load vertex shader (invalid working directory?)" << std::endl;
    return -1;
  }

  shaders.push_back(vertexShaderId);

  GLuint fragmentShaderId = loadShader("shaders/fragmentShader.glsl", GL_FRAGMENT_SHADER, &errorFlag);
  if(errorFlag) {
    std::cerr << "Failed to load fragment shader (invalid working directory?)" << std::endl;
    return -1;
  }
  shaders.push_back(fragmentShaderId);

  GLuint programId = prepareProgram(shaders, &errorFlag);
  if(errorFlag) {
    std::cerr << "Failed to prepare program" << std::endl;
    return -1;
  }
  glDeleteShader(vertexShaderId);
  glDeleteShader(fragmentShaderId);

  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(globVertexBufferData), globVertexBufferData, GL_STATIC_DRAW);

  GLuint vao;
  glGenVertexArrays(1, &vao);

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind VBO
  glBindVertexArray(0); // unbind VAO

  static const float speed = 0.1f; // units per second
  glm::vec3 position(0, 0, 5); // Camera is at (0, 0, 5)
  float horizontalAngleRad = 3.14f; // horizontal angle : toward -Z
  float verticalAngleRad = 0.0f; // vertical angle : 0, look at the horizon
  static const float mouseSpeedRad = 0.0025f;

  glm::mat4 projection = glm::perspective(90.0f, 4.0f / 3.0f, 0.3f, 100.0f);

  GLint matrixId = glGetUniformLocation(programId, "MVP");

  auto startTime = std::chrono::high_resolution_clock::now();

  // TODO: getViewMatrix
  // in: float deltaTimeMs, context: { horizontalAngleRad, verticalAngleRad, position, ... }
  // out: view metrix

  // hide cursor
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  while(glfwWindowShouldClose(window) == GL_FALSE) {
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto duration = currentTime - startTime;
    float deltaTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    float deltaTimeSec = deltaTimeMs/1000.0f;
    float rotationTimeMs = 3000.0f;
    float currentRotation = deltaTimeMs / rotationTimeMs;
    float angle = 360.0f*(currentRotation - (long)currentRotation);

    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);

    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    horizontalAngleRad += mouseSpeedRad * deltaTimeSec * static_cast<float>(windowWidth/2 - mouseX);
    verticalAngleRad   += mouseSpeedRad * deltaTimeSec * static_cast<float>(windowHeight/2 - mouseY);

    glfwSetCursorPos(window, windowWidth/2, windowHeight/2);

    // Direction : Spherical coordinates to Cartesian coordinates conversion
    glm::vec3 direction(
        cos(verticalAngleRad) * sin(horizontalAngleRad),
        sin(verticalAngleRad),
        cos(verticalAngleRad) * cos(horizontalAngleRad)
    );

    // Right vector
    glm::vec3 right = glm::vec3(
        sin(horizontalAngleRad - 3.14f/2.0f),
        0,
        cos(horizontalAngleRad - 3.14f/2.0f)
    );

    glm::vec3 up = glm::cross( right, direction );

    // Move forward
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
      position += direction * deltaTimeSec * speed;
    }
    // Move backward
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
      position -= direction * deltaTimeSec * speed;
    }
    // Strafe left
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
      position -= right * deltaTimeSec * speed;
    }
    // Strafe right
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
      position += right * deltaTimeSec * speed;
    }

    // Camera matrix
    glm::mat4 view = glm::lookAt(position, position + direction, up);
    glm::mat4 model = glm::rotate(angle, 0.0f, 1.0f, 0.0f);
    glm::mat4 mvp = projection * view * model; // Remember, matrix multiplication is the other way around
    glUniformMatrix4fv(matrixId, 1, GL_FALSE, &mvp[0][0]);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(programId);

    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisableVertexAttribArray(0);
    
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(1, &vbo);
  glDeleteProgram(programId);

  return 0;
}
