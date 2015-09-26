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

#define U(x) (x)
#define V(x) (1.0f - (x))

void windowSizeCallback(GLFWwindow *, int width, int height) {
  glViewport(0, 0, width, height);
}



// TODO: переименовать проект, поправить название в README.md

int main() {
//  bool saved = modelSave("models/box.emd", &globBoxVertexData, sizeof(globBoxVertexData), &globBoxIndices, sizeof(globBoxIndices));
//  std::cout << "Model saved = " << saved << std::endl;

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

  GLFWwindow* window = glfwCreateWindow(800, 600, "Skybox", nullptr, nullptr);
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


  unsigned int grassVerticesNumber;
  size_t grassVerticesBufferSize;
  GLfloat* grassVerticesBuffer = importedModelCreate("models/grass.blend", 0, &grassVerticesBufferSize, &grassVerticesNumber);
  if(grassVerticesBuffer == nullptr) return -1;
  defer(importedModelFree(grassVerticesBuffer));

  unsigned int skyboxVerticesNumber;
  size_t skyboxVerticesBufferSize;
  GLfloat* skyboxVerticesBuffer = importedModelCreate("models/skybox.blend", 0, &skyboxVerticesBufferSize, &skyboxVerticesNumber);
  if(skyboxVerticesBuffer == nullptr) return -1;
  defer(importedModelFree(skyboxVerticesBuffer));

  unsigned int towerVerticesNumber;
  size_t towerVerticesBufferSize;
  GLfloat* towerVerticesBuffer = importedModelCreate("models/tower.obj", 2, &towerVerticesBufferSize, &towerVerticesNumber);
  if(towerVerticesBuffer == nullptr) return -1;
  defer(importedModelFree(towerVerticesBuffer));


  // === prepare VBOs ===
  GLuint vboArray[5];
  int vbosNum = sizeof(vboArray)/sizeof(vboArray[0]);
  glGenBuffers(vbosNum, vboArray);
  defer(glDeleteBuffers(vbosNum, vboArray));

  GLuint boxVBO = vboArray[0];
  GLuint grassVBO = vboArray[1];
  GLuint skyboxVBO = vboArray[2];
  GLuint boxIndicesVBO = vboArray[3];
  GLuint towerVBO = vboArray[4];

  glBindBuffer(GL_ARRAY_BUFFER, grassVBO);
  glBufferData(GL_ARRAY_BUFFER, grassVerticesBufferSize, grassVerticesBuffer, GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
  glBufferData(GL_ARRAY_BUFFER, skyboxVerticesBufferSize, skyboxVerticesBuffer, GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, towerVBO);
  glBufferData(GL_ARRAY_BUFFER, towerVerticesBufferSize, towerVerticesBuffer, GL_STATIC_DRAW);

  // === prepare textures ===
  GLuint textureArray[4];
  int texturesNum = sizeof(textureArray)/sizeof(textureArray[0]);
  glGenTextures(texturesNum, textureArray);
  defer(glDeleteTextures(texturesNum, textureArray));

  GLuint boxTexture = textureArray[0];
  GLuint grassTexture = textureArray[1];
  GLuint skyboxTexture = textureArray[2];
  GLuint towerTexture = textureArray[3];

  if(!loadDDSTexture("textures/box.dds", boxTexture)) return -1;
  if(!loadDDSTexture("textures/grass.dds", grassTexture)) return -1;
  if(!loadDDSTexture("textures/skybox.dds", skyboxTexture)) return -1;

  // see http://gamedev.stackexchange.com/a/11975
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  if(!loadDDSTexture("textures/tower.dds", towerTexture)) return -1;

  // === prepare VAOs ===
  GLuint vaoArray[4];
  int vaosNum = sizeof(vaoArray)/sizeof(vaoArray[0]);
  glGenVertexArrays(vaosNum, vaoArray);
  defer(glDeleteVertexArrays(vaosNum, vaoArray));

  GLuint boxVAO = vaoArray[0];
  GLuint grassVAO = vaoArray[1];
  GLuint skyboxVAO = vaoArray[2];
  GLuint towerVAO = vaoArray[3];

  GLsizei boxIndicesNumber;
  GLenum boxIndexType;
  if(!modelLoad("models/box-v0.emd", boxVAO, boxVBO, boxIndicesVBO, &boxIndicesNumber, &boxIndexType)) return -1;

  glBindVertexArray(grassVAO);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, grassVBO);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), nullptr);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), (const void*)(3*sizeof(GLfloat)));

  glBindVertexArray(skyboxVAO);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), nullptr);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), (const void*)(3*sizeof(GLfloat)));

  glBindVertexArray(towerVAO);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, towerVBO);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), nullptr);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), (const void*)(3*sizeof(GLfloat)));

  glm::mat4 projection = glm::perspective(70.0f, 4.0f / 3.0f, 0.3f, 250.0f);

  GLint matrixId = glGetUniformLocation(programId, "MVP");
  GLint samplerId = glGetUniformLocation(programId, "textureSampler");

  auto startTime = std::chrono::high_resolution_clock::now();
  auto prevTime = startTime;

  // hide cursor
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

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

    glBindTexture(GL_TEXTURE_2D, boxTexture);

    glBindVertexArray(boxVAO);
    glm::mat4 boxMVP = vp * glm::rotate(islandAngle, 0.0f, 1.0f, 0.0f) * glm::translate(-10.0f, 0.0f, -10.0f);
    glUniformMatrix4fv(matrixId, 1, GL_FALSE, &boxMVP[0][0]);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, boxIndicesVBO);
    glDrawElements(GL_TRIANGLES, boxIndicesNumber, boxIndexType, nullptr);

    glBindTexture(GL_TEXTURE_2D, towerTexture);
    glBindVertexArray(towerVAO);
    glm::mat4 towerMVP = vp * glm::rotate(islandAngle, 0.0f, 1.0f, 0.0f) * glm::translate(-1.5f, -1.0f, -1.5f);
    glUniformMatrix4fv(matrixId, 1, GL_FALSE, &towerMVP[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, towerVerticesNumber);

    glBindTexture(GL_TEXTURE_2D, grassTexture);

    glBindVertexArray(grassVAO);
    glm::mat4 grassMVP = vp * glm::rotate(islandAngle, 0.0f, 1.0f, 0.0f) * glm::translate(0.0f, -1.0f, 0.0f);;
    glUniformMatrix4fv(matrixId, 1, GL_FALSE, &grassMVP[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, grassVerticesNumber);

    glBindTexture(GL_TEXTURE_2D, skyboxTexture);

    glm::vec3 cameraPos;
    camera.getPosition(&cameraPos);

    glBindVertexArray(skyboxVAO);
    glm::mat4 skyboxMatrix = glm::translate(cameraPos) * glm::scale(100.0f,100.0f,100.0f);
    glm::mat4 skyboxMVP = vp * skyboxMatrix;
    glUniformMatrix4fv(matrixId, 1, GL_FALSE, &skyboxMVP[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, skyboxVerticesNumber);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  return 0;
}
