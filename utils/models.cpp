#include <cstdint>
#include <cstdio>
#include <iostream>
#include <defer.h>
#include <cstring>
#include "utils.h"
#include "models.h"

#pragma pack(push, 1)

struct EaxmodHeader {
  char signature[7];
  unsigned char version;
  uint16_t headerSize;
  uint32_t verticesDataSize;
  uint32_t indicesDataSize;
};

#pragma pack(pop)

static const char eaxmodSignature[] = "EAXMOD";
static const char eaxmodVersion = 0;

bool checkFileSizeAndHeader(const char* fname, const EaxmodHeader * header, unsigned int fileSize) {
  if(fileSize < sizeof(EaxmodHeader)) {
    std::cout << "loadModel - file is too small, fname = " << fname << std::endl;
    return false;
  }

  if(strncmp(header->signature, eaxmodSignature, sizeof(eaxmodSignature)) != 0) {
    std::cout << "loadModel - invalid signature, fname = " << fname << std::endl;
    return false;
  }

  if(header->version != eaxmodVersion) {
    std::cout << "loadModel - unsupported version " << header->version << ", fname = " << fname << std::endl;
    return false;
  }

  if(sizeof(eaxmodSignature) > header->headerSize) {
    std::cout << "loadModel - invalid header size, actual: " << header->headerSize << ", expected at least: " << sizeof(eaxmodSignature) << ", fname = " << fname << std::endl;
    return false;
  }

  uint32_t expectedSize = header->headerSize + header->verticesDataSize + header->indicesDataSize;

  if(fileSize != expectedSize) {
    std::cout << "loadModel - invalid size, actual: " << fileSize << ", expected: " << expectedSize << ", fname = " << fname << std::endl;
    return false;
  }

  return true;
}

bool saveModel(const char* fname, const void* verticesData, size_t verticesDataSize, const void* indicesData, size_t indicesDataSize) {
  FILE* fd = fopen(fname, "wb");
  if(fd == nullptr) {
    std::cout << "saveModel - failed to open file, fname = " << fname << std::endl;
    return false;
  }
  defer(fclose(fd));

  EaxmodHeader header;
  strcpy(&(header.signature[0]), eaxmodSignature);
  header.version = eaxmodVersion;
  header.headerSize = sizeof(header);
  header.verticesDataSize = (uint32_t)verticesDataSize;
  header.indicesDataSize = (uint32_t)indicesDataSize;

  if(fwrite(&header, sizeof(header), 1, fd) != 1) {
    std::cout << "saveModel - failed to write header, fname = " << fname << std::endl;
    return false;
  }

  if(fwrite(verticesData, verticesDataSize, 1, fd) != 1) {
    std::cout << "saveModel - failed to write verticesData, fname = " << fname << std::endl;
    return false;
  }

  if(fwrite(indicesData, indicesDataSize, 1, fd) != 1) {
    std::cout << "saveModel - failed to write indicesData, fname = " << fname << std::endl;
    return false;
  }

  return true;
}

bool loadModel(const char* fname, GLuint modelVAO, GLuint modelVBO, GLuint indicesVBO) {
  FileMapping* mapping = fileMappingCreate(fname);
  if(mapping == nullptr) return false;
  defer(fileMappingClose(mapping));

  unsigned char* dataPtr = fileMappingGetPointer(mapping);
  unsigned int dataSize = fileMappingGetSize(mapping);

  EaxmodHeader * header = (EaxmodHeader *)dataPtr;
  if(!checkFileSizeAndHeader(fname, header, dataSize)) return false;

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
