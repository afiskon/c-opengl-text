#include <cstdint>
#include <cstdio>
#include <iostream>
#include <defer.h>
#include <cstring>
#include "utils.h"
#include "models.h"
#include "fileMapping.h"
#include "../assimp/include/assimp/Importer.hpp"
#include "../assimp/include/assimp/postprocess.h"
#include "../assimp/include/assimp/scene.h"

#pragma pack(push, 1)

struct EaxmodHeader {
  char signature[7];
  unsigned char version;
  uint16_t headerSize;
  uint32_t verticesDataSize;
  uint32_t indicesDataSize;
  unsigned char indexSize;
};

#pragma pack(pop)

static const char eaxmodSignature[] = "EAXMOD";
static const char eaxmodVersion = 1;

bool checkFileSizeAndHeader(const char* fname, const EaxmodHeader * header, unsigned int fileSize) {
  if(fileSize < sizeof(EaxmodHeader)) {
    std::cout << "modelLoad - file is too small, fname = " << fname << std::endl;
    return false;
  }

  if(strncmp(header->signature, eaxmodSignature, sizeof(eaxmodSignature)) != 0) {
    std::cout << "modelLoad - invalid signature, fname = " << fname << std::endl;
    return false;
  }

  if(header->version > eaxmodVersion) {
    std::cout << "modelLoad - unsupported version " << header->version << ", fname = " << fname << std::endl;
    return false;
  }

  if(sizeof(eaxmodSignature) > header->headerSize) {
    std::cout << "modelLoad - invalid header size, actual: " << header->headerSize << ", expected at least: " << sizeof(eaxmodSignature) << ", fname = " << fname << std::endl;
    return false;
  }

  uint32_t expectedSize = header->headerSize + header->verticesDataSize + header->indicesDataSize;
  if(fileSize != expectedSize) {
    std::cout << "modelLoad - invalid size, actual: " << fileSize << ", expected: " << expectedSize << ", fname = " << fname << std::endl;
    return false;
  }

  return true;
}

bool modelSave(const char *fname, const void *verticesData, size_t verticesDataSize, const void *indicesData,
               size_t indicesDataSize, unsigned char indexSize) {
  if(indexSize != 1 && indexSize != 2 && indexSize != 4) {
    std::cout << "modelSave - invalid index size " << indexSize << std::endl;
    return false;
  }

  FILE* fd = fopen(fname, "wb");
  if(fd == nullptr) {
    std::cout << "modelSave - failed to open file, fname = " << fname << std::endl;
    return false;
  }
  defer(fclose(fd));

  EaxmodHeader header;
  strcpy(&(header.signature[0]), eaxmodSignature);
  header.version = eaxmodVersion;
  header.headerSize = sizeof(header);
  header.verticesDataSize = (uint32_t)verticesDataSize;
  header.indicesDataSize = (uint32_t)indicesDataSize;
  header.indexSize = indexSize;

  if(fwrite(&header, sizeof(header), 1, fd) != 1) {
    std::cout << "modelSave - failed to write header, fname = " << fname << std::endl;
    return false;
  }

  if(fwrite(verticesData, verticesDataSize, 1, fd) != 1) {
    std::cout << "modelSave - failed to write verticesData, fname = " << fname << std::endl;
    return false;
  }

  if(fwrite(indicesData, indicesDataSize, 1, fd) != 1) {
    std::cout << "modelSave - failed to write indicesData, fname = " << fname << std::endl;
    return false;
  }

  return true;
}

bool modelLoad(const char *fname, GLuint modelVAO, GLuint modelVBO, GLuint indicesVBO, GLsizei* outIndicesNumber, GLenum* outIndicesType) {
  *outIndicesNumber = 0;
  *outIndicesType = GL_UNSIGNED_BYTE;
  unsigned char indexSize = 1; // default, for v0 format

  FileMapping* mapping = fileMappingCreate(fname);
  if(mapping == nullptr) return false;
  defer(fileMappingClose(mapping));

  unsigned char* dataPtr = fileMappingGetPointer(mapping);
  unsigned int dataSize = fileMappingGetSize(mapping);

  EaxmodHeader * header = (EaxmodHeader *)dataPtr;
  if(!checkFileSizeAndHeader(fname, header, dataSize)) return false;

  if(header->version > 0) {
    indexSize = header->indexSize;
    if(indexSize == 1) {
      *outIndicesType = GL_UNSIGNED_BYTE;
    } else if(indexSize == 2) {
      *outIndicesType = GL_UNSIGNED_SHORT;
    } else if(indexSize == 4) {
      *outIndicesType = GL_UNSIGNED_INT;
    } else {
      std::cout << "modelLoad - unsupported indexSize: " << indexSize << std::endl;
      return false;
    }
  }

  *outIndicesNumber = header->indicesDataSize / indexSize;

  unsigned char* verticesPtr = dataPtr + header->headerSize;
  unsigned char* indicesPtr = verticesPtr + header->verticesDataSize;

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesVBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, header->indicesDataSize, indicesPtr, GL_STATIC_DRAW);

  glBindVertexArray(modelVAO);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, modelVBO);
  glBufferData(GL_ARRAY_BUFFER, header->verticesDataSize, verticesPtr, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), nullptr);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), (const void*)(3*sizeof(GLfloat)));

  return true;
}

// TODO: import from .blend, join textures to one file, convert UV coordinates, etc...
GLfloat* importedModelCreate(const char* fname, unsigned int meshNumber, size_t* outVerticesBufferSize, unsigned int* outVerticesNumber) { // TODO: optimize + indices
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

  if(mesh->mTextureCoords == nullptr) {
    std::cerr << "mesh->mTextureCoords == nullptr, fname = " << fname << std::endl;
    return nullptr;
  }

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

void importedModelFree(GLfloat* model) {
  free(model);
}
