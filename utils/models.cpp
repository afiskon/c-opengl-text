#include <string.h>
#include <stdlib.h>
#include <stdio.h>
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

static const char eaxmodSignature[] = "EAXMOD";
static const char eaxmodVersion = 2;

static bool
checkFileSizeAndHeader(const char* fname, const EaxmodHeader * header,
					   unsigned int fileSize) {
  if(fileSize < sizeof(EaxmodHeader)) {
    fprintf(stderr, "modelLoad - file is too small, fname = %s\n", fname);
    return false;
  }

  if(strncmp(header->signature, eaxmodSignature, sizeof(eaxmodSignature)) != 0) {
    fprintf(stderr, "modelLoad - invalid signature, fname = %s\n", fname);
    return false;
  }

  if(header->version != eaxmodVersion) {
    fprintf(stderr, "modelLoad - unsupported version %d, fname = %s\n", (int)header->version, fname);
    return false;
  }

  if(sizeof(eaxmodSignature) > header->headerSize) {
    fprintf(stderr, "modelLoad - invalid header size, actual: %d, , expected at least: %d, fname = %s\n", (int)header->headerSize, (int)sizeof(eaxmodSignature), fname);
    return false;
  }

  uint32_t expectedSize = header->headerSize + header->verticesDataSize + header->indicesDataSize;
  if(fileSize != expectedSize) {
    fprintf(stderr, "modelLoad - invalid size, actual: %u, expected: %u, fname = %s\n", fileSize, expectedSize, fname);
    return false;
  }

  return true;
}

bool modelSave(const char *fname, const GLfloat *verticesData, size_t verticesDataSize, const unsigned int *indices, unsigned int indicesNumber) {
  unsigned char indexSize = 1;
  if(indicesNumber > 255) indexSize *= 2;
  if(indicesNumber > 65535) indexSize *= 2;

  void* indicesData = NULL;
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

  FILE* fd = fopen(fname, "wb");
  if(fd == NULL) {
    fprintf(stderr, "modelSave - failed to open file, fname = %s\n", fname);
    if(indicesData != (void*)indices) free(indicesData);
    return false;
  }

  EaxmodHeader header;
  strcpy(&(header.signature[0]), eaxmodSignature);
  header.version = eaxmodVersion;
  header.headerSize = sizeof(header);
  header.verticesDataSize = (uint32_t)verticesDataSize;
  header.indicesDataSize = (uint32_t)indicesDataSize;
  header.indexSize = indexSize;

  if(fwrite(&header, sizeof(header), 1, fd) != 1) {
    fprintf(stderr, "modelSave - failed to write header, fname = %s\n", fname);
    if(indicesData != (void*)indices) free(indicesData);
    fclose(fd);
    return false;
  }

  if(fwrite(verticesData, verticesDataSize, 1, fd) != 1) {
    fprintf(stderr, "modelSave - failed to write verticesData, fname = %s\n", fname);
    if(indicesData != (void*)indices) free(indicesData);
    fclose(fd);
    return false;
  }

  if(fwrite(indicesData, indicesDataSize, 1, fd) != 1) {
    fprintf(stderr, "modelSave - failed to write indicesData, fname = %s\n", fname);
    if(indicesData != (void*)indices) free(indicesData);
    fclose(fd);
    return false;
  }

  if(indicesData != (void*)indices) free(indicesData);
  fclose(fd);
  return true;
}

bool modelLoad(const char *fname, GLuint modelVAO, GLuint modelVBO, GLuint indicesVBO, GLsizei* outIndicesNumber, GLenum* outIndicesType) {
  *outIndicesNumber = 0;
  *outIndicesType = GL_UNSIGNED_BYTE;

  FileMapping* mapping = fileMappingCreate(fname);
  if(mapping == NULL) return false;

  unsigned char* dataPtr = fileMappingGetPointer(mapping);
  unsigned int dataSize = fileMappingGetSize(mapping);

  EaxmodHeader * header = (EaxmodHeader *)dataPtr;
  if(!checkFileSizeAndHeader(fname, header, dataSize)) {
    fileMappingDestroy(mapping);
    return false;
  } 

  unsigned char indexSize = header->indexSize;
  if(indexSize == 1) {
    *outIndicesType = GL_UNSIGNED_BYTE;
  } else if(indexSize == 2) {
    *outIndicesType = GL_UNSIGNED_SHORT;
  } else if(indexSize == 4) {
    *outIndicesType = GL_UNSIGNED_INT;
  } else {
    fprintf(stderr, "modelLoad - unsupported indexSize: %d, fname = %s\n",
      (int)indexSize, fname);
    fileMappingDestroy(mapping);
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
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride,
    NULL);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride,
    (const void*)(3*sizeof(GLfloat)));
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride,
    (const void*)(6*sizeof(GLfloat)));

  fileMappingDestroy(mapping);
  return true;
}
