#include <iostream>
#include <defer.h>
#include "fileMapping.h"

#ifdef _WIN32

#include <windows.h>

// TODO: under construction!

bool loadDDSTexture(char const *fname, GLuint textureId) {
  HANDLE hFile = CreateFile(fname, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
  if(hFile == INVALID_HANDLE_VALUE) {
    std::cout << "ERROR: loadDDSTexture - CreateFile failed, fname =  " << fname << std::endl;
    return false;
  }
  defer(CloseHandle(hFile));

  DWORD dwFileSize = GetFileSize(hFile, nullptr);
  if(dwFileSize == INVALID_FILE_SIZE) {
    std::cout << "ERROR: loadDDSTexture - GetFileSize failed, fname =  " << fname << std::endl;
    return false;
  }

  HANDLE hMapping = CreateFileMapping(hFile, nullptr, PAGE_READONLY, 0, 0, nullptr);
  if(hMapping == nullptr) { // yes, NULL, not INVALID_HANDLE_VALUE, see MSDN
    std::cout << "ERROR: loadDDSTexture - CreateFileMapping failed, fname =  " << fname << std::endl;
    return false;
  }
  defer(CloseHandle(hMapping));

  unsigned char* dataPtr = (unsigned char*)MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, dwFileSize);
  if(dataPtr == nullptr) {
    std::cout << "ERROR: loadDDSTexture - MapViewOfFile failed, fname =  " << fname << std::endl;
    return false;
  }
  defer(UnmapViewOfFile(dataPtr));

  return loadDDSTextureCommon(fname, textureId, (size_t)dwFileSize, dataPtr);
}

#else // Linux, MacOS, etc

#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>

struct FileMapping {
    int fd;
    size_t fsize;
    unsigned char* dataPtr;
};

FileMapping * fileMappingCreate(const char* fname) {
  int fd = open(fname, O_RDONLY, 0);
  if(fd < 0) {
    std::cout << "ERROR: fileMappingCreate - open failed, fname =  " << fname <<
    ", " << strerror(errno) << std::endl;
    return nullptr;
  }

  struct stat st;
  if(fstat(fd, &st) < 0) {
    std::cout << "ERROR: fileMappingCreate - fstat failed, fname = " << fname <<
    ", " << strerror(errno) << std::endl;
    close(fd);
    return nullptr;
  }

  size_t fsize = (size_t)st.st_size;

  unsigned char* dataPtr = (unsigned char*)mmap(nullptr, fsize, PROT_READ, MAP_PRIVATE, fd, 0);
  if(dataPtr == MAP_FAILED) {
    std::cout << "ERROR: fileMappingCreate - mmap failed, fname = " << fname <<
    ", " << strerror(errno) << std::endl;
    close(fd);
    return nullptr;
  }

  FileMapping * mapping = (FileMapping *)malloc(sizeof(FileMapping));
  mapping->fd = fd;
  mapping->fsize = fsize;
  mapping->dataPtr = dataPtr;

  return mapping;
}

unsigned char* fileMappingGetPointer(FileMapping * mapping) {
  return mapping->dataPtr;
}

unsigned int fileMappingGetSize(FileMapping * mapping) {
  return (unsigned int)mapping->fsize;
}

void fileMappingClose(FileMapping * mapping) {
  munmap(mapping->dataPtr, mapping->fsize);
  close(mapping->fd);
  free(mapping);
}

#endif