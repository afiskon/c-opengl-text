#include <GLXW/glxw.h>

#include <iostream>
#include <vector>
#include <defer.h>
#include "../assimp/include/assimp/Importer.hpp"
#include "../assimp/include/assimp/postprocess.h"
#include "../assimp/include/assimp/scene.h"
#include "utils/models.h"

static const unsigned int floatsPerVertex = (3 + 3 + 2); // 3 per position + 3 per normal + UV

GLfloat* importedModelCreate(const char* fname, unsigned int meshNumber, size_t* outVerticesBufferSize, unsigned int* outVerticesNumber) {
  *outVerticesBufferSize = 0;
  *outVerticesNumber = 0;
  Assimp::Importer importer;
  const aiScene* scene = importer.ReadFile(fname, aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);

  if(scene == nullptr) {
    std::cerr << "Failed to load model " << fname << std::endl;
    return nullptr;
  }

  if(scene->mNumMeshes <= meshNumber) {
    std::cerr << "There is no mesh #" << meshNumber << " in model (" << scene->mNumMeshes << " only), fname = " << fname << std::endl;
    return nullptr;
  }

  aiMesh* mesh = scene->mMeshes[meshNumber];
  unsigned int facesNum = mesh->mNumFaces;
//  unsigned int verticesNum = mesh->mNumVertices;

  if(mesh->mTextureCoords[0] == nullptr) {
    std::cerr << "mesh->mTextureCoords[0] == nullptr, fname = " << fname << std::endl;
    return nullptr;
  }

  unsigned int verticesPerFace = 3;
  *outVerticesNumber = facesNum*verticesPerFace;
  *outVerticesBufferSize = *outVerticesNumber * sizeof(GLfloat) * floatsPerVertex;
  GLfloat* verticesBuffer = (GLfloat*)malloc(*outVerticesBufferSize);

  unsigned int verticesBufferIndex = 0;

  for(unsigned int i = 0; i < facesNum; ++i) {
    const aiFace& face = mesh->mFaces[i];
    if(face.mNumIndices != verticesPerFace) {
      std::cerr << "face.numIndices = " << face.mNumIndices << " (3 expected), i = " << i << ", fname = " << fname << std::endl;
      free(verticesBuffer);
      return nullptr;
    }

    for(unsigned int j = 0; j < face.mNumIndices; ++j) {
      unsigned int index = face.mIndices[j];
      aiVector3D pos = mesh->mVertices[index];
      aiVector3D uv = mesh->mTextureCoords[0][index];
      aiVector3D normal = mesh->mNormals[index];

      std::cout << "DEBUG: normal = (" << normal.x << ", " << normal.y << ", " << normal.z << ")" << std::endl;

      verticesBuffer[verticesBufferIndex++] = pos.x;
      verticesBuffer[verticesBufferIndex++] = pos.y;
      verticesBuffer[verticesBufferIndex++] = pos.z;
      verticesBuffer[verticesBufferIndex++] = normal.x;
      verticesBuffer[verticesBufferIndex++] = normal.y;
      verticesBuffer[verticesBufferIndex++] = normal.z;
      verticesBuffer[verticesBufferIndex++] = uv.x;
      verticesBuffer[verticesBufferIndex++] = 1.0f - uv.y;
    }
  }

  return verticesBuffer;
}

bool importedModelSave(const char* fname, GLfloat* verticesBuffer, unsigned int verticesNumber) {
  std::vector<GLfloat> vertices;
  std::vector<unsigned int> indices;
  unsigned int usedIndices = 0;

  const GLfloat eps = 0.00005f; // real case: 1.0f and 0.999969f should be considered equal

  for(unsigned int vtx = 0; vtx < verticesNumber; ++vtx) {
    GLfloat currentPosX = verticesBuffer[vtx * floatsPerVertex + 0]; // TODO rewrite, use k index!
    GLfloat currentPosY = verticesBuffer[vtx * floatsPerVertex + 1];
    GLfloat currentPosZ = verticesBuffer[vtx * floatsPerVertex + 2];
    GLfloat currentNormX = verticesBuffer[vtx * floatsPerVertex + 3];
    GLfloat currentNormY = verticesBuffer[vtx * floatsPerVertex + 4];
    GLfloat currentNormZ = verticesBuffer[vtx * floatsPerVertex + 5];
    GLfloat currentU = verticesBuffer[vtx * floatsPerVertex + 6];
    GLfloat currentV = verticesBuffer[vtx * floatsPerVertex + 7];

    unsigned int foundIndex = 0;
    bool indexFound = false;
    for(unsigned int idx = 0; !indexFound && idx < usedIndices; ++idx) {
      GLfloat idxPosX = vertices[idx * floatsPerVertex + 0];
      GLfloat idxPosY = vertices[idx * floatsPerVertex + 1];
      GLfloat idxPosZ = vertices[idx * floatsPerVertex + 2];
      GLfloat idxNormX = vertices[idx * floatsPerVertex + 3];
      GLfloat idxNormY = vertices[idx * floatsPerVertex + 4];
      GLfloat idxNormZ = vertices[idx * floatsPerVertex + 5];
      GLfloat idxU = vertices[idx * floatsPerVertex + 6];
      GLfloat idxV = vertices[idx * floatsPerVertex + 7];

      if((fabs(currentPosX - idxPosX) < eps) && (fabs(currentPosY - idxPosY) < eps) && (fabs(currentPosZ - idxPosZ) < eps) &&
         (fabs(currentNormX - idxNormX) < eps) && (fabs(currentNormY - idxNormY) < eps) && (fabs(currentNormZ - idxNormZ) < eps) &&
         (fabs(currentU - idxU) < eps) && (fabs(currentV - idxV) < eps)) {
        foundIndex = idx;
        indexFound = true;
      }
    }

    if(!indexFound) {
      vertices.push_back(currentPosX);
      vertices.push_back(currentPosY);
      vertices.push_back(currentPosZ);
      vertices.push_back(currentNormX);
      vertices.push_back(currentNormY);
      vertices.push_back(currentNormZ);
      vertices.push_back(currentU);
      vertices.push_back(currentV);

      foundIndex = usedIndices;
      usedIndices++;
    }

    indices.push_back(foundIndex);
  }

  unsigned char indexSize = 1;
  if(verticesNumber > 255) indexSize *= 2;
  if(verticesNumber > 65535) indexSize *= 2;

  unsigned int modelSize = (unsigned int) (verticesNumber*floatsPerVertex*sizeof(GLfloat));
  unsigned int indexedModelSize = (unsigned int) (usedIndices*floatsPerVertex*sizeof(GLfloat) + verticesNumber*indexSize);
  float ratio = (float)indexedModelSize*100.0f / (float)modelSize;
  std::cout << "importedModelSave - fname = " << fname << ", verticesNumber = " << verticesNumber << ", usedIndices = " << usedIndices << std::endl;
  std::cout << "importedModelSave - modelSize = " << modelSize << ", indexedModelSize = " << indexedModelSize << ", ratio = " << ratio << " %" << std::endl;

  return modelSave(fname, vertices.data(), usedIndices* floatsPerVertex *sizeof(GLfloat), indices.data(), verticesNumber);
}

void importedModelFree(GLfloat* model) {
  free(model);
}

int main(int argc, char* argv[]) {
  if(argc < 2) {
    std::cout << "Usage: emdconv <input file> <output file> [mesh number]" << std::endl;
    return 1;
  }

  char* infile = argv[1];
  char* outfile = argv[2];

  unsigned int meshNumber = 0;
  if(argc > 3) {
    meshNumber = (unsigned int) atoi(argv[3]);
  }

  std::cout << "Infile: " << infile << std::endl;
  std::cout << "Outfile: " << outfile << std::endl;
  std::cout << "Mesh number: " << meshNumber << std::endl;

  unsigned int modelVerticesNumber;
  size_t modelVerticesBufferSize;
  std::cout << "DEBUG 1" << std::endl;
  GLfloat * modelVerticesBuffer = importedModelCreate(infile, meshNumber, &modelVerticesBufferSize, &modelVerticesNumber);
  std::cout << "DEBUG 2" << std::endl;
  if(modelVerticesBuffer == nullptr) {
    std::cerr << "importedModelCreate returned null" << std::endl;
    return 2;
  }
  defer(std::cout << "DEBUG - defer importedModelFree" << std::endl; importedModelFree(modelVerticesBuffer));

  std::cout << "DEBUG before imported modelSave" << std::endl;
  if(!importedModelSave(outfile, modelVerticesBuffer, modelVerticesNumber)) {
    std::cerr << "importedModelSave failed" << std::endl;
    return 3;
  }

  std::cout << "Done!" << std::endl;

  return 0;
}