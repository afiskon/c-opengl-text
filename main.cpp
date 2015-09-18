#include <GLXW/glxw.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <defer.h>
#include <chrono>

#define STBI_NO_GIF 1
#define STB_IMAGE_IMPLEMENTATION 1
#include "stb/stb_image.h"

#include "utils/utils.h"
#include "utils/camera.h"

#define U(x) (x)
#define V(x) (1.0f - x)

static const GLfloat globBoxVertexData[] = {
//   X     Y     Z       U        V
// front
     1.0f, 1.0f, 1.0f,   U(1.0f), V(1.0f),
    -1.0f, 1.0f, 1.0f,   U(0.0f), V(1.0f),
     1.0f,-1.0f, 1.0f,   U(1.0f), V(0.0f),
// front, |_ part
    -1.0f, 1.0f, 1.0f,   U(0.0f), V(1.0f),
    -1.0f,-1.0f, 1.0f,   U(0.0f), V(0.0f),
     1.0f,-1.0f, 1.0f,   U(1.0f), V(0.0f),
// left, _| part
    -1.0f,-1.0f,-1.0f,   U(0.0f), V(0.0f),
    -1.0f,-1.0f, 1.0f,   U(1.0f), V(0.0f),
    -1.0f, 1.0f, 1.0f,   U(1.0f), V(1.0f),
// left
    -1.0f,-1.0f,-1.0f,   U(0.0f), V(0.0f),
    -1.0f, 1.0f, 1.0f,   U(1.0f), V(1.0f),
    -1.0f, 1.0f,-1.0f,   U(0.0f), V(1.0f),
// back
     1.0f, 1.0f,-1.0f,   U(0.0f), V(1.0f),
    -1.0f,-1.0f,-1.0f,   U(1.0f), V(0.0f),
    -1.0f, 1.0f,-1.0f,   U(1.0f), V(1.0f),
// back, |_ part
     1.0f, 1.0f,-1.0f,   U(0.0f), V(1.0f),
     1.0f,-1.0f,-1.0f,   U(0.0f), V(0.0f),
    -1.0f,-1.0f,-1.0f,   U(1.0f), V(0.0f),
// right
     1.0f, 1.0f, 1.0f,   U(0.0f), V(1.0f),
     1.0f,-1.0f,-1.0f,   U(1.0f), V(0.0f),
     1.0f, 1.0f,-1.0f,   U(1.0f), V(1.0f),
// right, |_ part
     1.0f,-1.0f,-1.0f,   U(1.0f), V(0.0f),
     1.0f, 1.0f, 1.0f,   U(0.0f), V(1.0f),
     1.0f,-1.0f, 1.0f,   U(0.0f), V(0.0f),
// top
     1.0f, 1.0f, 1.0f,   U(1.0f), V(0.0f),
     1.0f, 1.0f,-1.0f,   U(1.0f), V(1.0f),
    -1.0f, 1.0f,-1.0f,   U(0.0f), V(1.0f),
// top, |_ part
     1.0f, 1.0f, 1.0f,   U(1.0f), V(0.0f),
    -1.0f, 1.0f,-1.0f,   U(0.0f), V(1.0f),
    -1.0f, 1.0f, 1.0f,   U(0.0f), V(0.0f),
// bottom, _| part
     1.0f,-1.0f, 1.0f,   U(1.0f), V(1.0f),
    -1.0f,-1.0f,-1.0f,   U(0.0f), V(0.0f),
     1.0f,-1.0f,-1.0f,   U(1.0f), V(0.0f),
// bottom
     1.0f,-1.0f, 1.0f,   U(1.0f), V(1.0f),
    -1.0f,-1.0f, 1.0f,   U(0.0f), V(1.0f),
    -1.0f,-1.0f,-1.0f,   U(0.0f), V(0.0f),
};

static const GLfloat globGrassVertexData[] = {
//   X     Y     Z       U         V
    10.0f,-1.0f,-10.0f,  U(10.0f), V(10.0f),
   -10.0f,-1.0f,-10.0f,  U(10.0f), V( 0.0f),
   -10.0f,-1.0f, 10.0f,  U( 0.0f), V( 0.0f),
    10.0f,-1.0f,-10.0f,  U(10.0f), V(10.0f),
   -10.0f,-1.0f, 10.0f,  U( 0.0f), V( 0.0f),
    10.0f,-1.0f, 10.0f,  U(10.0f), V( 0.0f),
};

static const GLfloat globSkyboxVertexData[] = {
//   X     Y     Z       U        V
    20.0f, 20.0f,-20.0f, U(1.0f), V(1.0f),
   -20.0f, 20.0f,-20.0f, U(0.0f), V(1.0f),
    20.0f,-20.0f,-20.0f, U(1.0f), V(0.0f),
};

void windowSizeCallback(GLFWwindow *, int width, int height) {
  glViewport(0, 0, width, height);
}

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
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  GLFWwindow* window = glfwCreateWindow(300, 300, "Rotating box", nullptr, nullptr);
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

  // === prepare VBOs ===
  GLuint vboArray[3];
  int vbosNum = sizeof(vboArray)/sizeof(vboArray[0]);
  glGenBuffers(vbosNum, vboArray);
  defer(glDeleteBuffers(vbosNum, vboArray));

  GLuint boxVBO = vboArray[0];
  GLuint grassVBO = vboArray[1];
  GLuint skyboxVBO = vboArray[2];

  glBindBuffer(GL_ARRAY_BUFFER, boxVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(globBoxVertexData), globBoxVertexData, GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, grassVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(globGrassVertexData), globGrassVertexData, GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(globSkyboxVertexData), globSkyboxVertexData, GL_STATIC_DRAW);

  // === prepare textures ===
  GLuint textureArray[3];
  int texturesNum = sizeof(textureArray)/sizeof(textureArray[0]);
  glGenTextures(texturesNum, textureArray);
  defer(glDeleteTextures(texturesNum, textureArray));

  GLuint boxTexture = textureArray[0];
  GLuint grassTexture = textureArray[1];
  GLuint skyboxTexture = textureArray[2];

//  if(!loadCommonTexture("textures/box.jpg", boxTexture)) return -1;
  if(!loadDDSTexture("textures/box-dxt3.dds", boxTexture)) return -1;
  if(!loadDDSTexture("textures/grass.dds", grassTexture)) return -1;
  if(!loadDDSTexture("textures/skybox-side.dds", skyboxTexture)) return -1;

  // === prepare VAOs ===
  GLuint vaoArray[3];
  int vaosNum = sizeof(vaoArray)/sizeof(vaoArray[0]);
  glGenVertexArrays(vaosNum, vaoArray);
  defer(glDeleteVertexArrays(vaosNum, vaoArray));

  GLuint boxVAO = vaoArray[0];
  GLuint grassVAO = vaoArray[1];
  GLuint skyboxVAO = vaoArray[2];

  glBindVertexArray(boxVAO);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, boxVBO);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), nullptr);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), (const void*)(3*sizeof(GLfloat)));
  // glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind VBO
  // glBindVertexArray(0); // unbind VAO

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

  glm::mat4 projection = glm::perspective(80.0f, 4.0f / 3.0f, 0.3f, 100.0f);

  GLint matrixId = glGetUniformLocation(programId, "MVP");
  GLint samplerId = glGetUniformLocation(programId, "textureSampler");

  auto startTime = std::chrono::high_resolution_clock::now();
  auto prevTime = startTime;

  // hide cursor
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  Camera camera(window, glm::vec3(0, 0, 5), 3.14f /* toward -Z */, 0.0f /* look at the horizon */);

  glEnable(GL_DOUBLEBUFFER);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glDepthFunc(GL_LESS);

  glClearColor(0, 0, 0, 1);

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

    float rotationTimeMs = 3000.0f;
    float currentRotation = startDeltaTimeMs / rotationTimeMs;
    float boxAngle = 360.0f*(currentRotation - (long)currentRotation);

    glm::mat4 view;
    camera.getViewMatrix(prevDeltaTimeMs, &view);
    glm::mat4 vp = projection * view;

    // TODO: move above
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, boxTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, grassTexture);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, skyboxTexture);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(programId);

    glBindVertexArray(boxVAO);
    glUniform1i(samplerId, 0);
    glm::mat4 boxMatrix = glm::rotate(boxAngle, 0.0f, 1.0f, 0.0f);
    glm::mat4 boxMVP = vp * boxMatrix;
    glUniformMatrix4fv(matrixId, 1, GL_FALSE, &boxMVP[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 3*12);

    glBindVertexArray(grassVAO);
    glUniform1i(samplerId, 1);
    glUniformMatrix4fv(matrixId, 1, GL_FALSE, &vp[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 3*2);

    glBindVertexArray(skyboxVAO);
    glUniform1i(samplerId, 2);
    glUniformMatrix4fv(matrixId, 1, GL_FALSE, &vp[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 3*1);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  return 0;
}
