#include <cstdint>
#include <cstdio>
#include <iostream>
#include <defer.h>
#include <cstring>
#include "models.h"

#pragma pack(push, 1)

struct eaxmodHeader {
  char signature[7]; // "EAXMOD\x00"
  unsigned char version;
  uint16_t headerSize;
  uint32_t verticesDataSize;
  uint32_t indicesDataSize;
};

#pragma pack(pop)

bool saveModel(const char* fname, const void* verticesData, size_t verticesDataSize, const void* indicesData, size_t indicesDataSize) {
  FILE* fd = fopen(fname, "wb");
  if(fd == nullptr) {
    std::cout << "saveModel - failed to open file, fname = " << fname << std::endl;
    return false;
  }
  defer(fclose(fd));

  eaxmodHeader header;
  strcpy(&(header.signature[0]), "EAXMOD");
  header.version = 0;
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