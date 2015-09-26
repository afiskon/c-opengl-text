#include <GLXW/glxw.h>

#include <iostream>
#include <vector>
#include <defer.h>
#include "../assimp/include/assimp/Importer.hpp"
#include "../assimp/include/assimp/postprocess.h"
#include "../assimp/include/assimp/scene.h"
#include "utils/models.h"

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
    std::cerr << "There is no mesh #" << meshNumber << " in model (" << scene->mNumMeshes << " only)" << fname << std::endl;
    return nullptr;
  }

  aiMesh* mesh = scene->mMeshes[meshNumber];
  unsigned int facesNum = mesh->mNumFaces;
//  unsigned int verticesNum = mesh->mNumVertices;

  *outVerticesNumber = facesNum*3;

  if(mesh->mTextureCoords[0] == nullptr) {
    std::cerr << "mesh->mTextureCoords[0] == nullptr, fname = " << fname << std::endl;
    return nullptr;
  }

  *outVerticesBufferSize = facesNum*sizeof(GLfloat)* 5 /* coordinates per vertex */ * 3 /* 3 vertices per face */;
  GLfloat* verticesBuffer = (GLfloat*)malloc(*outVerticesBufferSize);

  unsigned int verticesBufferIndex = 0;

  for(unsigned int i = 0; i < facesNum; ++i) {
    const aiFace& face = mesh->mFaces[i];
    if(face.mNumIndices != 3) {
      std::cerr << "face.numIndices = " << face.mNumIndices << " (3 expected), i = " << i << ", fname = " << fname << std::endl;
      free(verticesBuffer);
      return nullptr;
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

  return verticesBuffer;
}

bool importedModelSave(const char* fname, GLfloat* verticesBuffer, unsigned int verticesNumber) {
  std::vector<GLfloat> vertices;
  std::vector<unsigned int> indices;
  unsigned int usedIndices = 0;
  unsigned const int floatsPerVertex = 5; // 3 coordinates + UV

  const GLfloat eps = 0.00001f;

  for(unsigned int vtx = 0; vtx < verticesNumber; ++vtx) {
    GLfloat currentX = verticesBuffer[vtx* floatsPerVertex +0];
    GLfloat currentY = verticesBuffer[vtx* floatsPerVertex +1];
    GLfloat currentZ = verticesBuffer[vtx* floatsPerVertex +2];
    GLfloat currentU = verticesBuffer[vtx* floatsPerVertex +3];
    GLfloat currentV = verticesBuffer[vtx* floatsPerVertex +4];

    unsigned int foundIndex = 0;
    bool indexFound = false;
    for(unsigned int idx = 0; !indexFound && idx < usedIndices; ++idx) {
      GLfloat idxX = vertices[idx * floatsPerVertex + 0];
      GLfloat idxY = vertices[idx * floatsPerVertex + 1];
      GLfloat idxZ = vertices[idx * floatsPerVertex + 2];
      GLfloat idxU = vertices[idx * floatsPerVertex + 3];
      GLfloat idxV = vertices[idx * floatsPerVertex + 4];

      if((fabs(currentX - idxX) < eps) && (fabs(currentY - idxY) < eps) && (fabs(currentZ - idxZ) < eps) &&
         (fabs(currentU - idxU) < eps) && (fabs(currentV - idxV) < eps)) {
        foundIndex = idx;
        indexFound = true;
      }
    }

    if(!indexFound) {
      vertices.push_back(currentX);
      vertices.push_back(currentY);
      vertices.push_back(currentZ);
      vertices.push_back(currentU);
      vertices.push_back(currentV);

      foundIndex = usedIndices;
      usedIndices++;
    }

    indices.push_back(foundIndex);
  }

  return modelSave(fname, vertices.data(), usedIndices* floatsPerVertex *sizeof(GLfloat), indices.data(), verticesNumber);
}

void importedModelFree(GLfloat* model) {
  free(model);
}

int main(int argc, char* argv[]) {
  if(argc < 2) {
    std::cout << "Usage: emdconv <input file> <output file>" << std::endl;
    return 1;
  }

  char* infile = argv[1];
  char* outfile = argv[2];

  std::cout << "Infile: " << infile << std::endl;
  std::cout << "Outfile: " << outfile << std::endl;

  unsigned int modelVerticesNumber;
  size_t modelVerticesBufferSize;
  GLfloat * modelVerticesBuffer = importedModelCreate(infile, 0, &modelVerticesBufferSize, &modelVerticesNumber);
  if(modelVerticesBuffer == nullptr) {
    std::cerr << "importedModelCreate returned null" << std::endl;
    return 2;
  }
  defer(importedModelFree(modelVerticesBuffer));
  if(!importedModelSave(outfile, modelVerticesBuffer, modelVerticesNumber)) {
    std::cerr << "importedModelSave failed" << std::endl;
    return 3;
  }

  std::cout << "Done!" << std::endl;

  return 0;
}