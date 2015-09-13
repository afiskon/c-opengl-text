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
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include "utils/utils.h"
#include "utils/camera.h"

static const GLfloat globVertexBufferData[] = {
    1.0f, 1.0f, 1.0f, // front
    -1.0f, 1.0f, 1.0f,
    1.0f,-1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f, // front, |_ part
    -1.0f,-1.0f, 1.0f,
    1.0f,-1.0f, 1.0f,
    -1.0f,-1.0f,-1.0f, // left, _| part
    -1.0f,-1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f,-1.0f,-1.0f, // left
    -1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f,-1.0f,
    1.0f, 1.0f,-1.0f, // back
    -1.0f,-1.0f,-1.0f,
    -1.0f, 1.0f,-1.0f,
    1.0f, 1.0f,-1.0f, // back, |_ part
    1.0f,-1.0f,-1.0f,
    -1.0f,-1.0f,-1.0f,
    1.0f,-1.0f, 1.0f, // bottom, _| part
    -1.0f,-1.0f,-1.0f,
    1.0f,-1.0f,-1.0f,
    1.0f,-1.0f, 1.0f, // bottom
    -1.0f,-1.0f, 1.0f,
    -1.0f,-1.0f,-1.0f,
    1.0f, 1.0f, 1.0f, // right
    1.0f,-1.0f,-1.0f,
    1.0f, 1.0f,-1.0f,
    1.0f,-1.0f,-1.0f, // right, |_ part
    1.0f, 1.0f, 1.0f,
    1.0f,-1.0f, 1.0f,
    1.0f, 1.0f, 1.0f, // top
    1.0f, 1.0f,-1.0f,
    -1.0f, 1.0f,-1.0f,
    1.0f, 1.0f, 1.0f, // top, |_ part
    -1.0f, 1.0f,-1.0f,
    -1.0f, 1.0f, 1.0f,
};

// TODO: use ONE VBO
static const GLfloat globUVData[] = {
//    0.0f, 1.0f, 0.0f, // green
//    0.0f, 1.0f, 0.0f,
//    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f,
    1.0f, 0.0f,
    0.0f, 1.0f,


//    0.0f, 1.0f, 0.0f, // green
//    0.0f, 1.0f, 0.0f,
//    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f,
    1.0f, 0.0f,
    0.0f, 1.0f,

//    1.0f, 0.0f, 0.0f, // red
//    1.0f, 0.0f, 0.0f,
//    1.0f, 0.0f, 0.0f,
    0.0f, 0.0f,
    1.0f, 0.0f,
    0.0f, 1.0f,

//    1.0f, 0.0f, 0.0f, // red
//    1.0f, 0.0f, 0.0f,
//    1.0f, 0.0f, 0.0f,
    0.0f, 0.0f,
    1.0f, 0.0f,
    0.0f, 1.0f,

//    1.0f, 0.5f, 0.0f, // orange
//    1.0f, 0.5f, 0.0f,
//    1.0f, 0.5f, 0.0f,
    0.0f, 0.0f,
    1.0f, 0.0f,
    0.0f, 1.0f,

//    1.0f, 0.5f, 0.0f, // orange
//    1.0f, 0.5f, 0.0f,
//    1.0f, 0.5f, 0.0f,
    0.0f, 0.0f,
    1.0f, 0.0f,
    0.0f, 1.0f,

//    1.0f, 0.0f, 1.0f, // violet
//    1.0f, 0.0f, 1.0f,
//    1.0f, 0.0f, 1.0f,
    0.0f, 0.0f,
    1.0f, 0.0f,
    0.0f, 1.0f,

//    1.0f, 0.0f, 1.0f, // violet
//    1.0f, 0.0f, 1.0f,
//    1.0f, 0.0f, 1.0f,
    0.0f, 0.0f,
    1.0f, 0.0f,
    0.0f, 1.0f,
//    1.0f, 1.0f, 0.0f, // yellow
//    1.0f, 1.0f, 0.0f,
//    1.0f, 1.0f, 0.0f,
    0.0f, 0.0f,
    1.0f, 0.0f,
    0.0f, 1.0f,
//    1.0f, 1.0f, 0.0f, // yellow
//    1.0f, 1.0f, 0.0f,
//    1.0f, 1.0f, 0.0f,
    0.0f, 0.0f,
    1.0f, 0.0f,
    0.0f, 1.0f,
//    0.0f, 0.0f, 1.0f, // blue
//    0.0f, 0.0f, 1.0f,
//    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f,
    1.0f, 0.0f,
    0.0f, 1.0f,

//    0.0f, 0.0f, 1.0f, // blue
//    0.0f, 0.0f, 1.0f,
//    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f,
    1.0f, 0.0f,
    0.0f, 1.0f,
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

  GLFWwindow* window = glfwCreateWindow(800, 600, "Rotating cube", nullptr, nullptr);
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

  GLuint vertexVBO;
  glGenBuffers(1, &vertexVBO);
  defer(glDeleteBuffers(1, &vertexVBO));

  glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(globVertexBufferData), globVertexBufferData, GL_STATIC_DRAW);

  GLuint uvVBO;
  glGenBuffers(1, &uvVBO);
  defer(glDeleteBuffers(1, &uvVBO));

  glBindBuffer(GL_ARRAY_BUFFER, uvVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(globUVData), globUVData, GL_STATIC_DRAW);


  // Create one OpenGL texture
  GLuint texture;
  glGenTextures(1, &texture);
  defer(glDeleteTextures(1, &texture));

// "Bind" the newly created texture : all future texture functions will modify this texture
  glBindTexture(GL_TEXTURE_2D, texture);

  int width, height, n;
  unsigned char *textureData = stbi_load("textures/box.jpg", &width, &height, &n, 0);
  if(textureData == nullptr) {
    std::cout << "Failed to load a texture!" << std::endl;
    return -1;
  }
  defer(stbi_image_free(textureData));

  std::cout << "Texture loaded! Width = " << width << ", height = " << height << ", n = " << n << std::endl;

  // Give the image to OpenGL
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // TODO: fix GL_NEAREST + how to process 1.5, 1.5 case
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  glBindTexture(GL_TEXTURE_2D, 0); // unbind

  GLuint vao;
  glGenVertexArrays(1, &vao);
  defer(glDeleteVertexArrays(1, &vao));

  glBindVertexArray(vao);

  glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind VBO

  glBindBuffer(GL_ARRAY_BUFFER, uvVBO);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
  glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind VBO

  glBindVertexArray(0); // unbind VAO

  glm::mat4 projection = glm::perspective(80.0f, 4.0f / 3.0f, 0.3f, 100.0f);

  GLint matrixId = glGetUniformLocation(programId, "MVP");
  GLint textureId = glGetUniformLocation(programId, "textureSampler");

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
    float angle = 360.0f*(currentRotation - (long)currentRotation);

    glm::mat4 view;
    camera.getViewMatrix(prevDeltaTimeMs, &view);

    glm::mat4 model = glm::rotate(angle, 0.0f, 1.0f, 0.0f);
    glm::mat4 mvp = projection * view * model; // matrix multiplication is the other way around
    glUniformMatrix4fv(matrixId, 1, GL_FALSE, &mvp[0][0]);

    // Bind our texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Set our "textureSampler" sampler to user Texture Unit 0
    glUniform1i(textureId, 0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(programId);

    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glDrawArrays(GL_TRIANGLES, 0, 3*12);
    glEnableVertexAttribArray(1);
    glDisableVertexAttribArray(0);
    
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  return 0;
}
