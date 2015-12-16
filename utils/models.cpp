#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <defer.h>
#include <cstring>
#include "utils.h"
#include "models.h"
#include "fileMapping.h"

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

// TODO: replace std::cout with printf

static const char eaxmodSignature[] = "EAXMOD";
static const char eaxmodVersion = 2;

static bool
checkFileSizeAndHeader(const char* fname, const EaxmodHeader * header,
					   unsigned int fileSize) {
  if(fileSize < sizeof(EaxmodHeader)) {
    std::cerr << "modelLoad - file is too small, fname = " << fname << std::endl;
    return false;
  }

  if(strncmp(header->signature, eaxmodSignature, sizeof(eaxmodSignature)) != 0) {
    std::cerr << "modelLoad - invalid signature, fname = " << fname << std::endl;
    return false;
  }

  if(header->version != eaxmodVersion) {
    std::cerr << "modelLoad - unsupported version " << (int)header->version << ", fname = " << fname << std::endl;
    return false;
  }

  if(sizeof(eaxmodSignature) > header->headerSize) {
    std::cerr << "modelLoad - invalid header size, actual: " << header->headerSize << ", expected at least: " << sizeof(eaxmodSignature) << ", fname = " << fname << std::endl;
    return false;
  }

  uint32_t expectedSize = header->headerSize + header->verticesDataSize + header->indicesDataSize;
  if(fileSize != expectedSize) {
    std::cerr << "modelLoad - invalid size, actual: " << fileSize << ", expected: " << expectedSize << ", fname = " << fname << std::endl;
    return false;
  }

  return true;
}

bool modelSave(const char *fname, const GLfloat *verticesData, size_t verticesDataSize, const unsigned int *indices, unsigned int indicesNumber) {
  unsigned char indexSize = 1;
  if(indicesNumber > 255) indexSize *= 2;
  if(indicesNumber > 65535) indexSize *= 2;

  void* indicesData = nullptr;
  uint32_t indicesDataSize = indicesNumber*indexSize;

  if(indexSize == sizeof(unsigned int)) {
    indicesData = (void*)indices;
  } else if(indexSize == sizeof(unsigned short)) {
    indicesData = malloc(indicesDataSize);
    unsigned short *copyToPtr = (unsigned short*)indicesData;
    const unsigned int *copyFromPtr = indices;
    for(unsigned int i = 0; i < indicesNumber; ++i) {
      *copyToPtr = (unsigned short)*copyFromPtr;
      copyToPtr++;
      copyFromPtr++;
    }
  } else { // if(indexSize == sizeof(unsigned char))
    indicesData = malloc(indicesDataSize);
    unsigned char *copyToPtr = (unsigned char*)indicesData;
    const unsigned int *copyFromPtr = indices;
    for(unsigned int i = 0; i < indicesNumber; ++i) {
      *copyToPtr = (unsigned char)*copyFromPtr;
      copyToPtr++;
      copyFromPtr++;
    }
  }

  defer(if(indicesData != (void*)indices){ free(indicesData); });

  FILE* fd = fopen(fname, "wb");
  if(fd == nullptr) {
    std::cerr << "modelSave - failed to open file, fname = " << fname << std::endl;
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
    std::cerr << "modelSave - failed to write header, fname = " << fname << std::endl;
    return false;
  }

  if(fwrite(verticesData, verticesDataSize, 1, fd) != 1) {
    std::cerr << "modelSave - failed to write verticesData, fname = " << fname << std::endl;
    return false;
  }

  if(fwrite(indicesData, indicesDataSize, 1, fd) != 1) {
    std::cerr << "modelSave - failed to write indicesData, fname = " << fname << std::endl;
    return false;
  }

  return true;
}

bool modelLoad(const char *fname, GLuint modelVAO, GLuint modelVBO, GLuint indicesVBO, GLsizei* outIndicesNumber, GLenum* outIndicesType) {
  *outIndicesNumber = 0;
  *outIndicesType = GL_UNSIGNED_BYTE;

  FileMapping* mapping = fileMappingCreate(fname);
  if(mapping == nullptr) return false;
  defer(fileMappingClose(mapping));

  unsigned char* dataPtr = fileMappingGetPointer(mapping);
  unsigned int dataSize = fileMappingGetSize(mapping);

  EaxmodHeader * header = (EaxmodHeader *)dataPtr;
  if(!checkFileSizeAndHeader(fname, header, dataSize)) return false;

  unsigned char indexSize = header->indexSize;
  if(indexSize == 1) {
    *outIndicesType = GL_UNSIGNED_BYTE;
  } else if(indexSize == 2) {
    *outIndicesType = GL_UNSIGNED_SHORT;
  } else if(indexSize == 4) {
    *outIndicesType = GL_UNSIGNED_INT;
  } else {
    std::cerr << "modelLoad - unsupported indexSize: " << indexSize << std::endl;
    return false;
  }

  *outIndicesNumber = header->indicesDataSize / indexSize;

  unsigned char* verticesPtr = dataPtr + header->headerSize;
  unsigned char* indicesPtr = verticesPtr + header->verticesDataSize;

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesVBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, header->indicesDataSize, indicesPtr, GL_STATIC_DRAW);

  glBindVertexArray(modelVAO);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);

  glBindBuffer(GL_ARRAY_BUFFER, modelVBO);
  glBufferData(GL_ARRAY_BUFFER, header->verticesDataSize, verticesPtr, GL_STATIC_DRAW);

  GLsizei stride = 8*sizeof(GLfloat);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, nullptr);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (const void*)(3*sizeof(GLfloat)));
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (const void*)(6*sizeof(GLfloat)));

  return true;
}
