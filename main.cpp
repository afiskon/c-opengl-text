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

void setupLights(GLuint programId, bool directionalLightEnabled, bool pointLightEnabled, bool spotLightEnabled) {
  {
    glm::vec3 direction = glm::normalize(glm::vec3(0.0f, -1.0f, 1.0f));

    setUniform3f(programId, "directionalLight.direction", direction.x, direction.y, direction.z);
    setUniform3f(programId, "directionalLight.color", 1.0f, 1.0f, 1.0f);
    setUniform1f(programId, "directionalLight.ambientIntensity", float(directionalLightEnabled)*0.1f);
    setUniform1f(programId, "directionalLight.diffuseIntensity", float(directionalLightEnabled)*0.1f);
    setUniform1f(programId, "directionalLight.specularIntensity", float(directionalLightEnabled)*1.0f);
  }

  {
    setUniform3f(programId, "pointLight.position", -2.0f, 3.0f, 0.0f);
    setUniform3f(programId, "pointLight.color", 1.0f, 0.0f, 0.0f);
    setUniform1f(programId, "pointLight.ambientIntensity", float(pointLightEnabled) * 0.2f);
    setUniform1f(programId, "pointLight.diffuseIntensity", float(pointLightEnabled) * 1.0f);
    setUniform1f(programId, "pointLight.specularIntensity", float(pointLightEnabled) * 1.0f);
  }

  {
    glm::vec3 direction = glm::normalize(glm::vec3(-1.0f, -1.0f, 0.0f));

    setUniform3f(programId, "spotLight.direction", direction.x, direction.y, direction.z);
    setUniform3f(programId, "spotLight.position", 2.5f, 1.0f, 0.0f);
    setUniform1f(programId, "spotLight.cutoff", glm::cos(glm::radians(25.0f)));
    setUniform3f(programId, "spotLight.color", 0.0f, 0.0f, 1.0f);
    setUniform1f(programId, "spotLight.ambientIntensity", float(spotLightEnabled)*0.2f);
    setUniform1f(programId, "spotLight.diffuseIntensity", float(spotLightEnabled)*10.0f);
    setUniform1f(programId, "spotLight.specularIntensity", float(spotLightEnabled)*1.0f);
  }
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

  GLint uniformMVP = getUniformLocation(programId, "MVP");
  GLint uniformM = getUniformLocation(programId, "M");
  GLint uniformTextureSample = getUniformLocation(programId, "textureSampler");
  GLint uniformCameraPos = getUniformLocation(programId, "cameraPos");

  GLint uniformMaterialSpecularFactor = getUniformLocation(programId, "materialSpecularFactor");
  GLint uniformMaterialSpecularIntensity = getUniformLocation(programId, "materialSpecularIntensity");

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

  glUseProgram(programId);

  glUniform1i(uniformTextureSample, 0);

  bool directionalLightEnabled = true;
  bool pointLightEnabled = true;
  bool spotLightEnabled = true;
  float lastLightsChangeMs = 0.0;

  setupLights(programId, directionalLightEnabled, pointLightEnabled, spotLightEnabled);

  while(glfwWindowShouldClose(window) == GL_FALSE) {
    if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) break;
    if(glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    if(glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    auto currentTime = std::chrono::high_resolution_clock::now();
    float startDeltaTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count();
    float prevDeltaTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - prevTime).count();
    prevTime = currentTime;

    float rotationTimeMs = 100000.0f;
    float currentRotation = startDeltaTimeMs / rotationTimeMs;
    float islandAngle = 360.0f*(currentRotation - (long)currentRotation);

    if((glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) && (startDeltaTimeMs - lastLightsChangeMs > 300.0f)) {
      directionalLightEnabled = !directionalLightEnabled;
      lastLightsChangeMs = startDeltaTimeMs;
      setupLights(programId, directionalLightEnabled, pointLightEnabled, spotLightEnabled);
    }
    if((glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) && (startDeltaTimeMs - lastLightsChangeMs > 300.0f)) {
      pointLightEnabled = !pointLightEnabled;
      lastLightsChangeMs = startDeltaTimeMs;
      setupLights(programId, directionalLightEnabled, pointLightEnabled, spotLightEnabled);
    }
    if((glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) && (startDeltaTimeMs - lastLightsChangeMs > 300.0f)) {
      spotLightEnabled = !spotLightEnabled;
      lastLightsChangeMs = startDeltaTimeMs;
      setupLights(programId, directionalLightEnabled, pointLightEnabled, spotLightEnabled);
    }

    glm::vec3 cameraPos;
    camera.getPosition(&cameraPos);

    glUniform3f(uniformCameraPos, cameraPos.x, cameraPos.y, cameraPos.z);

    glm::mat4 view;
    camera.getViewMatrix(prevDeltaTimeMs, &view);
    glm::mat4 vp = projection * view;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 towerM = glm::rotate(islandAngle, 0.0f, 1.0f, 0.0f) * glm::translate(-1.5f, -1.0f, -1.5f);
    glm::mat4 towerMVP = vp * towerM;

    glBindTexture(GL_TEXTURE_2D, towerTexture);
    glBindVertexArray(towerVAO);
    glUniformMatrix4fv(uniformMVP, 1, GL_FALSE, &towerMVP[0][0]);
    glUniformMatrix4fv(uniformM, 1, GL_FALSE, &towerM[0][0]);
    glUniform1f(uniformMaterialSpecularFactor, 1.0f);
    glUniform1f(uniformMaterialSpecularIntensity, 0.0f);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, towerIndicesVBO);
    glDrawElements(GL_TRIANGLES, towerIndicesNumber, towerIndexType, nullptr);

    glm::mat4 grassM = glm::rotate(islandAngle, 0.0f, 1.0f, 0.0f) * glm::translate(0.0f, -1.0f, 0.0f);
    glm::mat4 grassMVP = vp * grassM;

    glBindTexture(GL_TEXTURE_2D, grassTexture);
    glBindVertexArray(grassVAO);
    glUniformMatrix4fv(uniformMVP, 1, GL_FALSE, &grassMVP[0][0]);
    glUniformMatrix4fv(uniformM, 1, GL_FALSE, &grassM[0][0]);
    glUniform1f(uniformMaterialSpecularFactor, 32.0f);
    glUniform1f(uniformMaterialSpecularIntensity, 2.0f);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, grassIndicesVBO);
    glDrawElements(GL_TRIANGLES, grassIndicesNumber, grassIndexType, nullptr);

    glm::mat4 skyboxM = glm::translate(cameraPos) * glm::scale(100.0f,100.0f,100.0f);
    glm::mat4 skyboxMVP = vp * skyboxM;

    // TODO: implement modelDraw procedure (or maybe Model object?)
    glBindTexture(GL_TEXTURE_2D, skyboxTexture);
    glBindVertexArray(skyboxVAO);
    glUniformMatrix4fv(uniformMVP, 1, GL_FALSE, &skyboxMVP[0][0]);
    glUniformMatrix4fv(uniformM, 1, GL_FALSE, &skyboxM[0][0]);
    glUniform1f(uniformMaterialSpecularFactor, 1.0f);
    glUniform1f(uniformMaterialSpecularIntensity, 0.0f);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxIndicesVBO);
    glDrawElements(GL_TRIANGLES, skyboxIndicesNumber, skyboxIndexType, nullptr);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  return 0;
}
