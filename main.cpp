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
#include "assimp/include/assimp/Importer.hpp"
#include "assimp/include/assimp/postprocess.h"
#include "assimp/include/assimp/scene.h"

#define U(x) (x)
#define V(x) (1.0f - (x))

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
//   X    Y     Z     U         V
    1.0f, 1.0f,-1.0f, U(0.5f),  V(2.0f/3.0f),
   -1.0f, 1.0f,-1.0f, U(0.25f), V(2.0f/3.0f),
    1.0f,-1.0f,-1.0f, U(0.5f),  V(1.0f/3.0f),

   -1.0f, 1.0f,-1.0f, U(0.25f), V(2.0f/3.0f),
   -1.0f,-1.0f,-1.0f, U(0.25f), V(1.0f/3.0f),
    1.0f,-1.0f,-1.0f, U(0.5f),  V(1.0f/3.0f),

    1.0f,-1.0f,-1.0f, U(0.5f),  V(1.0f/3.0f),
    1.0f,-1.0f, 1.0f, U(0.75f), V(1.0f/3.0f),
    1.0f, 1.0f, 1.0f, U(0.75f), V(2.0f/3.0f),

    1.0f,-1.0f,-1.0f, U(0.5f),  V(1.0f/3.0f),
    1.0f, 1.0f, 1.0f, U(0.75f), V(2.0f/3.0f),
    1.0f, 1.0f,-1.0f, U(0.5f),  V(2.0f/3.0f),

   -1.0f,-1.0f, 1.0f, U(0.0f),  V(1.0f/3.0f),
   -1.0f,-1.0f,-1.0f, U(0.25f), V(1.0f/3.0f),
   -1.0f, 1.0f, 1.0f, U(0.0f),  V(2.0f/3.0f),

   -1.0f, 1.0f, 1.0f, U(0.0f),  V(2.0f/3.0f),
   -1.0f,-1.0f,-1.0f, U(0.25f), V(1.0f/3.0f),
   -1.0f, 1.0f,-1.0f, U(0.25f), V(2.0f/3.0f),

    1.0f,-1.0f, 1.0f, U(0.75f), V(1.0f/3.0f),
   -1.0f, 1.0f, 1.0f, U(1.0f),  V(2.0f/3.0f),
    1.0f, 1.0f, 1.0f, U(0.75f), V(2.0f/3.0f),

    1.0f,-1.0f, 1.0f, U(0.75f), V(1.0f/3.0f),
   -1.0f,-1.0f, 1.0f, U(1.0f),  V(1.0f/3.0f),
   -1.0f, 1.0f, 1.0f, U(1.0f),  V(2.0f/3.0f),

   -1.0f, 1.0f, 1.0f, U(0.25f), V(1.0f),
   -1.0f, 1.0f,-1.0f, U(0.25f), V(2.0f/3.0f),
    1.0f, 1.0f,-1.0f, U(0.5f),  V(2.0f/3.0f),

    1.0f, 1.0f,-1.0f, U(0.5f),  V(2.0f/3.0f),
    1.0f, 1.0f, 1.0f, U(0.5f),  V(1.0f),
   -1.0f, 1.0f, 1.0f, U(0.25f), V(1.0f),

    1.0f,-1.0f,-1.0f, U(0.5f),  V(0.0f),
   -1.0f,-1.0f,-1.0f, U(0.25f), V(0.0f),
   -1.0f,-1.0f, 1.0f, U(0.25f), V(1.0f/3.0f),

   -1.0f,-1.0f, 1.0f, U(0.25f), V(1.0f/3.0f),
    1.0f,-1.0f, 1.0f, U(0.5f),  V(1.0f/3.0f),
    1.0f,-1.0f,-1.0f, U(0.5f),  V(0.0f),
};

void windowSizeCallback(GLFWwindow *, int width, int height) {
  glViewport(0, 0, width, height);
}

// TODO: нарисовать ящик заново в Blender, написать конвертер, загрузить
// TODO: перерисовать остальные модели (оптимизировать skybox, особенно текстуру) и также заимпортить
// TODO: переписать класс сamera на обычные struct
// TODO: избавиться от схемы с errorFlagPtr
// TODO: пройтись по исходникам, заменить std::cout на std::cerr для вывода ошибок

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


  // TODO: move to modelImport procedure
  // TODO: implement modelOptimize procedure (used in modelImport)

  // ----------------------------------------
  Assimp::Importer importer;
  const char* fname = "models/monkey-textured.obj";
  const aiScene* scene = importer.ReadFile(fname, aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);

  if(scene == nullptr) {
    std::cerr << "Failed to load model " << fname << std::endl;
    return -1;
  }

  if(scene->mNumMeshes <= 0) {
    std::cerr << "No meshes in model " << fname << std::endl;
    return -1;
  }

  aiMesh* mesh = scene->mMeshes[0];
  unsigned int facesNum = mesh->mNumFaces;
//  unsigned int verticesNum = mesh->mNumVertices;

  unsigned int monkeyVerticesNumber = facesNum*3; // used below

  if(mesh->mTextureCoords == nullptr) {
    std::cerr << "mesh->mTextureCoords == nullptr, fname = " << fname << std::endl;
    return -1;
  }

  if(mesh->mTextureCoords[0] == nullptr) {
    std::cerr << "mesh->mTextureCoords[0] == nullptr, fname = " << fname << std::endl;
    return -1;
  }

  size_t verticesBufferSize = facesNum*sizeof(GLfloat)* 5 /* coordinates per vertex */ * 3 /* 3 vertices per face */;
  GLfloat* verticesBuffer = (GLfloat*)malloc(verticesBufferSize);
  defer(free(verticesBuffer));

  unsigned int verticesBufferIndex = 0;

  for(unsigned int i = 0; i < facesNum; ++i) {
    const aiFace& face = mesh->mFaces[i];
    if(face.mNumIndices != 3) {
      std::cerr << "face.numIndices = " << face.mNumIndices << " (3 expected), i = " << i << ", fname = " << fname << std::endl;
      return -1;
    }

    for(unsigned int j = 0; j < face.mNumIndices; ++j) {
      unsigned int index = face.mIndices[j];
      aiVector3D pos = mesh->mVertices[index];
      aiVector3D uv = mesh->mTextureCoords[0][index];
//      aiVector3D normal = mesh->mNormals[index];

      verticesBuffer[verticesBufferIndex++] = pos.x;
      verticesBuffer[verticesBufferIndex++] = pos.y;
      verticesBuffer[verticesBufferIndex++] = pos.z;
      verticesBuffer[verticesBufferIndex++] = uv.x;
      verticesBuffer[verticesBufferIndex++] = 1.0f - uv.y;
    }
  }

  // ----------------------------------------


  // === prepare VBOs ===
  GLuint vboArray[5];
  int vbosNum = sizeof(vboArray)/sizeof(vboArray[0]);
  glGenBuffers(vbosNum, vboArray);
  defer(glDeleteBuffers(vbosNum, vboArray));

  GLuint boxVBO = vboArray[0];
  GLuint grassVBO = vboArray[1];
  GLuint skyboxVBO = vboArray[2];
  GLuint boxIndicesVBO = vboArray[3];
  GLuint monkeyVBO = vboArray[4];

  glBindBuffer(GL_ARRAY_BUFFER, grassVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(globGrassVertexData), globGrassVertexData, GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(globSkyboxVertexData), globSkyboxVertexData, GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, monkeyVBO);
  glBufferData(GL_ARRAY_BUFFER, verticesBufferSize, verticesBuffer, GL_STATIC_DRAW);

  // === prepare textures ===
  GLuint textureArray[4];
  int texturesNum = sizeof(textureArray)/sizeof(textureArray[0]);
  glGenTextures(texturesNum, textureArray);
  defer(glDeleteTextures(texturesNum, textureArray));

  GLuint boxTexture = textureArray[0];
  GLuint grassTexture = textureArray[1];
  GLuint skyboxTexture = textureArray[2];
  GLuint monkeyTexture = textureArray[3];

  if(!loadDDSTexture("textures/box.dds", boxTexture)) return -1;
  if(!loadDDSTexture("textures/grass.dds", grassTexture)) return -1;
  if(!loadDDSTexture("textures/skybox.dds", skyboxTexture)) return -1;
  if(!loadDDSTexture("textures/monkey-texture.dds", monkeyTexture)) return -1;

  // see http://gamedev.stackexchange.com/a/11975
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  // === prepare VAOs ===
  GLuint vaoArray[4];
  int vaosNum = sizeof(vaoArray)/sizeof(vaoArray[0]);
  glGenVertexArrays(vaosNum, vaoArray);
  defer(glDeleteVertexArrays(vaosNum, vaoArray));

  GLuint boxVAO = vaoArray[0];
  GLuint grassVAO = vaoArray[1];
  GLuint skyboxVAO = vaoArray[2];
  GLuint monkeyVAO = vaoArray[3];

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

  glBindVertexArray(monkeyVAO);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, monkeyVBO);
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
    glm::mat4 boxMVP = vp * glm::rotate(islandAngle, 0.0f, 1.0f, 0.0f) * glm::translate(-2.0f, 0.0f, -3.0f);
    glUniformMatrix4fv(matrixId, 1, GL_FALSE, &boxMVP[0][0]);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, boxIndicesVBO);
    glDrawElements(GL_TRIANGLES, boxIndicesNumber, boxIndexType, nullptr);

    glm::mat4 monkeyMVP = boxMVP * glm::translate(0.0f, 2.0f, 0.0f);
    glBindTexture(GL_TEXTURE_2D, monkeyTexture);
    glBindVertexArray(monkeyVAO);
    glUniformMatrix4fv(matrixId, 1, GL_FALSE, &monkeyMVP[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, monkeyVerticesNumber);

    glBindTexture(GL_TEXTURE_2D, grassTexture);

    glBindVertexArray(grassVAO);
    glm::mat4 grassMVP = vp * glm::rotate(islandAngle, 0.0f, 1.0f, 0.0f);
    glUniformMatrix4fv(matrixId, 1, GL_FALSE, &grassMVP[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 3*2);

    glBindTexture(GL_TEXTURE_2D, skyboxTexture);

    glm::vec3 cameraPos;
    camera.getPosition(&cameraPos);

    glBindVertexArray(skyboxVAO);
    glm::mat4 skyboxMatrix = glm::translate(cameraPos) * glm::scale(100.0f,100.0f,100.0f);
    glm::mat4 skyboxMVP = vp * skyboxMatrix;
    glUniformMatrix4fv(matrixId, 1, GL_FALSE, &skyboxMVP[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 3*12);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  return 0;
}
