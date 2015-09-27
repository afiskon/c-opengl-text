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
#include "utils/camera.h"
#include "utils/models.h"

void windowSizeCallback(GLFWwindow *, int width, int height) {
  glViewport(0, 0, width, height);
}

int main() {
  if(glfwInit() == GL_FALSE) {
    std::cerr << "Failed to initialize GLFW" << std::endl;
    return -1;
  }
  defer(glfwTerminate());

  glfwDefaultWindowHints();

  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_SAMPLES, 4);

  GLFWwindow* window = glfwCreateWindow(800, 600, "Models", nullptr, nullptr);
  if(window == nullptr) {
    std::cerr << "Failed to open GLFW window" << std::endl;
    return -1;
  }
  defer(glfwDestroyWindow(window));

  glfwMakeContextCurrent(window);

  if(glxwInit()) {
    std::cerr << "Failed to init GLXW" << std::endl;
    return -1;
  }

  glfwSwapInterval(1);
  glfwSetWindowSizeCallback(window, windowSizeCallback);
  glfwShowWindow(window);

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
  defer(glDeleteProgram(programId));

  glDeleteShader(vertexShaderId);
  glDeleteShader(fragmentShaderId);

  // === prepare textures ===
  GLuint textureArray[3];
  int texturesNum = sizeof(textureArray)/sizeof(textureArray[0]);
  glGenTextures(texturesNum, textureArray);
  defer(glDeleteTextures(texturesNum, textureArray));

  GLuint grassTexture = textureArray[0];
  GLuint skyboxTexture = textureArray[1];
  GLuint towerTexture = textureArray[2];

  if(!loadDDSTexture("textures/grass.dds", grassTexture)) return -1;

  if(!loadDDSTexture("textures/skybox.dds", skyboxTexture)) return -1;
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  if(!loadDDSTexture("textures/tower.dds", towerTexture)) return -1;

  // === prepare VAOs ===
  GLuint vaoArray[3];
  int vaosNum = sizeof(vaoArray)/sizeof(vaoArray[0]);
  glGenVertexArrays(vaosNum, vaoArray);
  defer(glDeleteVertexArrays(vaosNum, vaoArray));

  GLuint grassVAO = vaoArray[0];
  GLuint skyboxVAO = vaoArray[1];
  GLuint towerVAO = vaoArray[2];

  // === prepare VBOs ===
  GLuint vboArray[6];
  int vbosNum = sizeof(vboArray)/sizeof(vboArray[0]);
  glGenBuffers(vbosNum, vboArray);
  defer(glDeleteBuffers(vbosNum, vboArray));

  GLuint grassVBO = vboArray[0];
  GLuint skyboxVBO = vboArray[1];
  GLuint towerVBO = vboArray[2];
  GLuint grassIndicesVBO = vboArray[3];
  GLuint skyboxIndicesVBO = vboArray[4];
  GLuint towerIndicesVBO = vboArray[5];

  // === load models ===

  GLsizei grassIndicesNumber, skyboxIndicesNumber, towerIndicesNumber;
  GLenum grassIndexType, skyboxIndexType, towerIndexType;
  if(!modelLoad("models/grass.emd", grassVAO, grassVBO, grassIndicesVBO, &grassIndicesNumber, &grassIndexType)) return -1;
  if(!modelLoad("models/skybox.emd", skyboxVAO, skyboxVBO, skyboxIndicesVBO, &skyboxIndicesNumber, &skyboxIndexType)) return -1;
  if(!modelLoad("models/tower.emd", towerVAO, towerVBO, towerIndicesVBO, &towerIndicesNumber, &towerIndexType)) return -1;

  glm::mat4 projection = glm::perspective(70.0f, 4.0f / 3.0f, 0.3f, 250.0f);

  GLint matrixId = glGetUniformLocation(programId, "MVP");
  GLint samplerId = glGetUniformLocation(programId, "textureSampler");

  auto startTime = std::chrono::high_resolution_clock::now();
  auto prevTime = startTime;

  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // hide cursor

  Camera camera(window, glm::vec3(0, 0, 5), 3.14f /* toward -Z */, 0.0f /* look at the horizon */);

  glEnable(GL_DOUBLEBUFFER);
  glEnable(GL_CULL_FACE);
  glEnable(GL_MULTISAMPLE);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  glClearColor(0, 0, 0, 1);

  glUniform1i(samplerId, 0);

  while(glfwWindowShouldClose(window) == GL_FALSE) {
    if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) break;

    if(glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    if(glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    auto currentTime = std::chrono::high_resolution_clock::now();
    float startDeltaTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count();
    float prevDeltaTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - prevTime).count();
    prevTime = currentTime;

    float rotationTimeMs = 100000.0f;
    float currentRotation = startDeltaTimeMs / rotationTimeMs;
    float islandAngle = 360.0f*(currentRotation - (long)currentRotation);

    glm::mat4 view;
    camera.getViewMatrix(prevDeltaTimeMs, &view);
    glm::mat4 vp = projection * view;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(programId);

    glm::mat4 towerMVP = vp * glm::rotate(islandAngle, 0.0f, 1.0f, 0.0f) * glm::translate(-1.5f, -1.0f, -1.5f);

    glBindTexture(GL_TEXTURE_2D, towerTexture);
    glBindVertexArray(towerVAO);
    glUniformMatrix4fv(matrixId, 1, GL_FALSE, &towerMVP[0][0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, towerIndicesVBO);
    glDrawElements(GL_TRIANGLES, towerIndicesNumber, towerIndexType, nullptr);

    glm::mat4 grassMVP = vp * glm::rotate(islandAngle, 0.0f, 1.0f, 0.0f) * glm::translate(0.0f, -1.0f, 0.0f);

    glBindTexture(GL_TEXTURE_2D, grassTexture);
    glBindVertexArray(grassVAO);
    glUniformMatrix4fv(matrixId, 1, GL_FALSE, &grassMVP[0][0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, grassIndicesVBO);
    glDrawElements(GL_TRIANGLES, grassIndicesNumber, grassIndexType, nullptr);

    glm::vec3 cameraPos;
    camera.getPosition(&cameraPos);
    glm::mat4 skyboxMatrix = glm::translate(cameraPos) * glm::scale(100.0f,100.0f,100.0f);
    glm::mat4 skyboxMVP = vp * skyboxMatrix;

    // TODO: implement modelDraw procedure (or maybe Model object?)
    glBindTexture(GL_TEXTURE_2D, skyboxTexture);
    glBindVertexArray(skyboxVAO);
    glUniformMatrix4fv(matrixId, 1, GL_FALSE, &skyboxMVP[0][0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxIndicesVBO);
    glDrawElements(GL_TRIANGLES, skyboxIndicesNumber, skyboxIndexType, nullptr);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  return 0;
}
