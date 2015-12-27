#include <stdio.h>
#include <defer.h>
#include "fileMapping.h"

#ifdef _WIN32

#include <windows.h>

struct FileMapping {
  HANDLE hFile;
  HANDLE hMapping;
  size_t fsize;
  unsigned char* dataPtr;
};

FileMapping * fileMappingCreate(const char* fname) {
  HANDLE hFile = CreateFile(fname, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
  if(hFile == INVALID_HANDLE_VALUE) {
    fprintf(stderr, "fileMappingCreate - CreateFile failed, fname =  %s\n", fname);
    return nullptr;
  }

  DWORD dwFileSize = GetFileSize(hFile, nullptr);
  if(dwFileSize == INVALID_FILE_SIZE) {
    fprintf(stderr, "fileMappingCreate - GetFileSize failed, fname =  %s\n", fname);
    CloseHandle(hFile);
    return nullptr;
  }

  HANDLE hMapping = CreateFileMapping(hFile, nullptr, PAGE_READONLY, 0, 0, nullptr);
  if(hMapping == nullptr) { // yes, NULL, not INVALID_HANDLE_VALUE, see MSDN
    fprintf(stderr, "fileMappingCreate - CreateFileMapping failed, fname =  %s\n", fname);
    CloseHandle(hFile);
    return nullptr;
  }

  unsigned char* dataPtr = (unsigned char*)MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, dwFileSize);
  if(dataPtr == nullptr) {
    fprintf(stderr, "fileMappingCreate - MapViewOfFile failed, fname =  %s\n", fname);
    CloseHandle(hMapping);
    CloseHandle(hFile);
    return nullptr;
  }

  FileMapping* mapping = (FileMapping*)malloc(sizeof(FileMapping));
  if(mapping == nullptr) {
    fprintf(stderr, "fileMappingCreate - malloc failed, fname = %s\n", fname);
    UnmapViewOfFile(dataPtr);
    CloseHandle(hMapping);
    CloseHandle(hFile);
    return nullptr;
  }

  mapping->hFile = hFile;
  mapping->hMapping = hMapping;
  mapping->dataPtr = dataPtr;
  mapping->fsize = (size_t)dwFileSize;

  return mapping;
}

void fileMappingDestroy(FileMapping* mapping) {
  UnmapViewOfFile(mapping->dataPtr);
  CloseHandle(mapping->hMapping);
  CloseHandle(mapping->hFile);
  free(mapping);
}

#else // Linux, MacOS, etc

#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

struct FileMapping {
    int fd;
    size_t fsize;
    unsigned char* dataPtr;
};

FileMapping * fileMappingCreate(const char* fname) {
  int fd = open(fname, O_RDONLY, 0);
  if(fd < 0) {
    fprintf(stderr, "fileMappingCreate - open failed, fname =  %s, strerror = %s\n", fname, strerror(errno));
    return nullptr;
  }

  struct stat st;
  if(fstat(fd, &st) < 0) {
    fprintf(stderr, "fileMappingCreate - fstat failed, fname = %s, strerror = %s\n", fname, strerror(errno));
    close(fd);
    return nullptr;
  }

  size_t fsize = (size_t)st.st_size;

  unsigned char* dataPtr = (unsigned char*)mmap(nullptr, fsize, PROT_READ, MAP_PRIVATE, fd, 0);
  if(dataPtr == MAP_FAILED) {
    fprintf(stderr, "fileMappingCreate - mmap failed, fname = %s, strerror = %s\n", fname, strerror(errno));
    close(fd);
    return nullptr;
  }

  FileMapping * mapping = (FileMapping *)malloc(sizeof(FileMapping));
  if(mapping == nullptr) {
    fprintf(stderr, "fileMappingCreate - malloc failed, fname = %s\n", fname);
    munmap(dataPtr, fsize);
    close(fd);
    return nullptr;
  }

  mapping->fd = fd;
  mapping->fsize = fsize;
  mapping->dataPtr = dataPtr;

  return mapping;
}

void fileMappingDestroy(FileMapping * mapping) {
  munmap(mapping->dataPtr, mapping->fsize);
  close(mapping->fd);
  free(mapping);
}

#endif

unsigned char* fileMappingGetPointer(FileMapping * mapping) {
  return mapping->dataPtr;
}

unsigned int fileMappingGetSize(FileMapping * mapping) {
  return (unsigned int)mapping->fsize;
}