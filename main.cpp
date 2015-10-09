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

static const glm::vec3 pointLightPos(-2.0f, 3.0f, 0.0f);
static const glm::vec3 spotLightPos(4.0f, 5.0f, 0.0f);

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
    setUniform3f(programId, "pointLight.position", pointLightPos.x, pointLightPos.y, pointLightPos.z);
    setUniform3f(programId, "pointLight.color", 1.0f, 0.0f, 0.0f);
    setUniform1f(programId, "pointLight.ambientIntensity", float(pointLightEnabled) * 0.1f);
    setUniform1f(programId, "pointLight.diffuseIntensity", float(pointLightEnabled) * 1.0f);
    setUniform1f(programId, "pointLight.specularIntensity", float(pointLightEnabled) * 1.0f);
  }

  {
    glm::vec3 direction = glm::normalize(glm::vec3(-0.5f, -1.0f, 0.0f));

    setUniform3f(programId, "spotLight.direction", direction.x, direction.y, direction.z);
    setUniform3f(programId, "spotLight.position", spotLightPos.x, spotLightPos.y, spotLightPos.z);
    setUniform1f(programId, "spotLight.cutoff", glm::cos(glm::radians(15.0f)));
    setUniform3f(programId, "spotLight.color", 0.0f, 0.0f, 1.0f);
    setUniform1f(programId, "spotLight.ambientIntensity", float(spotLightEnabled)*0.1f);
    setUniform1f(programId, "spotLight.diffuseIntensity", float(spotLightEnabled)*20.0f);
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
  GLuint textureArray[6];
  int texturesNum = sizeof(textureArray)/sizeof(textureArray[0]);
  glGenTextures(texturesNum, textureArray);
  defer(glDeleteTextures(texturesNum, textureArray));

  GLuint grassTexture = textureArray[0];
  GLuint skyboxTexture = textureArray[1];
  GLuint towerTexture = textureArray[2];
  GLuint garkGreenTexture = textureArray[3];
  GLuint redTexture = textureArray[4];
  GLuint blueTexture = textureArray[5];

  if(!loadDDSTexture("textures/grass.dds", grassTexture)) return -1;

  if(!loadDDSTexture("textures/skybox.dds", skyboxTexture)) return -1;
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  if(!loadDDSTexture("textures/tower.dds", towerTexture)) return -1;

  loadOneColorTexture(0.05f, 0.5f, 0.1f, garkGreenTexture);
  loadOneColorTexture(1.0f, 0.0f, 0.0f, redTexture);
  loadOneColorTexture(0.0f, 0.0f, 1.0f, blueTexture);

  // === prepare VAOs ===
  GLuint vaoArray[5];
  int vaosNum = sizeof(vaoArray)/sizeof(vaoArray[0]);
  glGenVertexArrays(vaosNum, vaoArray);
  defer(glDeleteVertexArrays(vaosNum, vaoArray));

  GLuint grassVAO = vaoArray[0];
  GLuint skyboxVAO = vaoArray[1];
  GLuint towerVAO = vaoArray[2];
  GLuint torusVAO = vaoArray[3];
  GLuint sphereVAO = vaoArray[4];

  // === prepare VBOs ===
  GLuint vboArray[10];
  int vbosNum = sizeof(vboArray)/sizeof(vboArray[0]);
  glGenBuffers(vbosNum, vboArray);
  defer(glDeleteBuffers(vbosNum, vboArray));

  GLuint grassVBO = vboArray[0];
  GLuint grassIndicesVBO = vboArray[1];
  GLuint skyboxVBO = vboArray[2];
  GLuint skyboxIndicesVBO = vboArray[3];
  GLuint towerVBO = vboArray[4];
  GLuint towerIndicesVBO = vboArray[5];
  GLuint torusVBO = vboArray[6];
  GLuint torusIndicesVBO = vboArray[7];
  GLuint sphereVBO = vboArray[8];
  GLuint sphereIndicesVBO = vboArray[9];
  // === load models ===

  GLsizei grassIndicesNumber, skyboxIndicesNumber, towerIndicesNumber, torusIndicesNumber, sphereIndicesNumber;
  GLenum grassIndexType, skyboxIndexType, towerIndexType, torusIndexType, sphereIndexType;
  if(!modelLoad("models/grass.emd", grassVAO, grassVBO, grassIndicesVBO, &grassIndicesNumber, &grassIndexType)) return -1;
  if(!modelLoad("models/skybox.emd", skyboxVAO, skyboxVBO, skyboxIndicesVBO, &skyboxIndicesNumber, &skyboxIndexType)) return -1;
  if(!modelLoad("models/tower.emd", towerVAO, towerVBO, towerIndicesVBO, &towerIndicesNumber, &towerIndexType)) return -1;
  if(!modelLoad("models/torus.emd", torusVAO, torusVBO, torusIndicesVBO, &torusIndicesNumber, &torusIndexType)) return -1;
  if(!modelLoad("models/sphere.emd", sphereVAO, sphereVBO, sphereIndicesVBO, &sphereIndicesNumber, &sphereIndexType)) return -1;

  glm::mat4 projection = glm::perspective(70.0f, 4.0f / 3.0f, 0.3f, 250.0f);

  GLint uniformMVP = getUniformLocation(programId, "MVP");
  GLint uniformM = getUniformLocation(programId, "M");
  GLint uniformTextureSample = getUniformLocation(programId, "textureSampler");
  GLint uniformCameraPos = getUniformLocation(programId, "cameraPos");

  GLint uniformMaterialSpecularFactor = getUniformLocation(programId, "materialSpecularFactor");
  GLint uniformMaterialSpecularIntensity = getUniformLocation(programId, "materialSpecularIntensity");
  GLint uniformMaterialEmission = getUniformLocation(programId, "materialEmission");

  auto startTime = std::chrono::high_resolution_clock::now();
  auto prevTime = startTime;

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
  bool wireframesModeEnabled = false;
  float lastKeyPressCheck = 0.0;
  const float keyPressCheckIntervalMs = 250.0f;

  setupLights(programId, directionalLightEnabled, pointLightEnabled, spotLightEnabled);

  while(glfwWindowShouldClose(window) == GL_FALSE) {
    if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) break;

    auto currentTime = std::chrono::high_resolution_clock::now();
    float startDeltaTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count();
    float prevDeltaTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - prevTime).count();
    prevTime = currentTime;

    float rotationTimeMs = 100000.0f;
    float currentRotation = startDeltaTimeMs / rotationTimeMs;
    float islandAngle = 360.0f*(currentRotation - (long)currentRotation);

    if(startDeltaTimeMs - lastKeyPressCheck > keyPressCheckIntervalMs) {
      if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
        lastKeyPressCheck = startDeltaTimeMs;
        wireframesModeEnabled = !wireframesModeEnabled;
        if (wireframesModeEnabled) {
          glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
          glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
      }
      if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
        lastKeyPressCheck = startDeltaTimeMs;
        bool enabled = camera.getMouseInterception();
        camera.setMouseInterception(!enabled);
      }
      if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
        lastKeyPressCheck = startDeltaTimeMs;
        directionalLightEnabled = !directionalLightEnabled;
        setupLights(programId, directionalLightEnabled, pointLightEnabled, spotLightEnabled);
      }
      if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
        lastKeyPressCheck = startDeltaTimeMs;
        pointLightEnabled = !pointLightEnabled;
        setupLights(programId, directionalLightEnabled, pointLightEnabled, spotLightEnabled);
      }
      if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
        lastKeyPressCheck = startDeltaTimeMs;
        spotLightEnabled = !spotLightEnabled;
        setupLights(programId, directionalLightEnabled, pointLightEnabled, spotLightEnabled);
      }
    }

    glm::vec3 cameraPos;
    camera.getPosition(&cameraPos);

    glUniform3f(uniformCameraPos, cameraPos.x, cameraPos.y, cameraPos.z);

    glm::mat4 view;
    camera.getViewMatrix(prevDeltaTimeMs, &view);
    glm::mat4 vp = projection * view;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // TODO implement ModelLoader and Model classes
    // tower

    glm::mat4 towerM = glm::rotate(islandAngle, 0.0f, 1.0f, 0.0f) * glm::translate(-1.5f, -1.0f, -1.5f);
    glm::mat4 towerMVP = vp * towerM;

    glBindTexture(GL_TEXTURE_2D, towerTexture);
    glBindVertexArray(towerVAO);
    glUniformMatrix4fv(uniformMVP, 1, GL_FALSE, &towerMVP[0][0]);
    glUniformMatrix4fv(uniformM, 1, GL_FALSE, &towerM[0][0]);
    glUniform1f(uniformMaterialSpecularFactor, 1.0f);
    glUniform1f(uniformMaterialSpecularIntensity, 0.0f);
    glUniform3f(uniformMaterialEmission, 0.0f, 0.0f, 0.0f);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, towerIndicesVBO);
    glDrawElements(GL_TRIANGLES, towerIndicesNumber, towerIndexType, nullptr);

    // torus

    glm::mat4 torusM = glm::translate(0.0f, 1.0f, 0.0f) * glm::rotate(-3*islandAngle, 0.0f, 0.5f, 0.0f);
    glm::mat4 torusMVP = vp * torusM;

    glBindTexture(GL_TEXTURE_2D, garkGreenTexture);
    glBindVertexArray(torusVAO);
    glUniformMatrix4fv(uniformMVP, 1, GL_FALSE, &torusMVP[0][0]);
    glUniformMatrix4fv(uniformM, 1, GL_FALSE, &torusM[0][0]);
    glUniform1f(uniformMaterialSpecularFactor, 1.0f);
    glUniform1f(uniformMaterialSpecularIntensity, 1.0f);
    glUniform3f(uniformMaterialEmission, 0.0f, 0.0f, 0.0f);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, torusIndicesVBO);
    glDrawElements(GL_TRIANGLES, torusIndicesNumber, torusIndexType, nullptr);

    // grass

    glm::mat4 grassM = glm::rotate(islandAngle, 0.0f, 1.0f, 0.0f) * glm::translate(0.0f, -1.0f, 0.0f);
    glm::mat4 grassMVP = vp * grassM;

    glBindTexture(GL_TEXTURE_2D, grassTexture);
    glBindVertexArray(grassVAO);
    glUniformMatrix4fv(uniformMVP, 1, GL_FALSE, &grassMVP[0][0]);
    glUniformMatrix4fv(uniformM, 1, GL_FALSE, &grassM[0][0]);
    glUniform1f(uniformMaterialSpecularFactor, 32.0f);
    glUniform1f(uniformMaterialSpecularIntensity, 2.0f);
    glUniform3f(uniformMaterialEmission, 0.0f, 0.0f, 0.0f);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, grassIndicesVBO);
    glDrawElements(GL_TRIANGLES, grassIndicesNumber, grassIndexType, nullptr);

    // skybox

    glm::mat4 skyboxM = glm::translate(cameraPos) * glm::scale(100.0f,100.0f,100.0f);
    glm::mat4 skyboxMVP = vp * skyboxM;

    glBindTexture(GL_TEXTURE_2D, skyboxTexture);
    glBindVertexArray(skyboxVAO);
    glUniformMatrix4fv(uniformMVP, 1, GL_FALSE, &skyboxMVP[0][0]);
    glUniformMatrix4fv(uniformM, 1, GL_FALSE, &skyboxM[0][0]);
    glUniform1f(uniformMaterialSpecularFactor, 1.0f);
    glUniform1f(uniformMaterialSpecularIntensity, 0.0f);
    glUniform3f(uniformMaterialEmission, 0.0f, 0.0f, 0.0f);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxIndicesVBO);
    glDrawElements(GL_TRIANGLES, skyboxIndicesNumber, skyboxIndexType, nullptr);

    // point light source
    if(pointLightEnabled) {
      glm::mat4 pointLightM = glm::translate(pointLightPos);
      glm::mat4 pointLightMVP = vp * pointLightM;

      glBindTexture(GL_TEXTURE_2D, redTexture);
      glBindVertexArray(sphereVAO);
      glUniformMatrix4fv(uniformMVP, 1, GL_FALSE, &pointLightMVP[0][0]);
      glUniformMatrix4fv(uniformM, 1, GL_FALSE, &pointLightM[0][0]);
      glUniform1f(uniformMaterialSpecularFactor, 1.0f);
      glUniform1f(uniformMaterialSpecularIntensity, 1.0f);
      glUniform3f(uniformMaterialEmission, 0.5f, 0.5f, 0.5f);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereIndicesVBO);
      glDrawElements(GL_TRIANGLES, sphereIndicesNumber, sphereIndexType, nullptr);
    }

    // spot light source
    if(spotLightEnabled) {
      glm::mat4 spotLightM = glm::translate(spotLightPos);
      glm::mat4 spotLightMVP = vp * spotLightM;

      glBindTexture(GL_TEXTURE_2D, blueTexture);
      glBindVertexArray(sphereVAO);
      glUniformMatrix4fv(uniformMVP, 1, GL_FALSE, &spotLightMVP[0][0]);
      glUniformMatrix4fv(uniformM, 1, GL_FALSE, &spotLightM[0][0]);
      glUniform1f(uniformMaterialSpecularFactor, 1.0f);
      glUniform1f(uniformMaterialSpecularIntensity, 1.0f);
      glUniform3f(uniformMaterialEmission, 0.5f, 0.5f, 0.5f);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereIndicesVBO);
      glDrawElements(GL_TRIANGLES, sphereIndicesNumber, sphereIndexType, nullptr);
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  return 0;
}
