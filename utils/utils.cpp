#include "utils.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <sstream>
#include <fstream>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <defer.h>
#include <unistd.h>
#include <stb_image.h>

void flipTexture(unsigned char *textureData, int width, int height, int n) {
  for(int h = 0; h < height/2; ++h) {
    for(int w = 0; w < width; ++w) {
      for(int i = 0; i < n; ++i) {
        int offset1 = (h*width + w)*n + i;
        int offset2 = ((height - h - 1)*width + w)*n + i;
        unsigned char t = textureData[offset1];
        textureData[offset1] = textureData[offset2];
        textureData[offset2] = t;
      }
    }
  }
}

bool loadCommonTexture(char const* fname, GLuint* texturePtr) {
  return loadCommonTextureExt(fname, texturePtr, true);
}

bool loadCommonTextureExt(char const* fname, GLuint* texturePtr, bool flip) {
  *texturePtr = 0;

  int width, height, n;
  unsigned char *textureData = stbi_load(fname, &width, &height, &n, 0);
  if(textureData == nullptr) {
    std::cout << "ERROR: loadCommonTextureExt failed, fname = " << fname << ", stbi_load returned nullptr" << std::endl;
    return false;
  }
  defer(stbi_image_free(textureData));

  if(flip) flipTexture(textureData, width, height, n);

  glGenTextures(1, texturePtr);
  glBindTexture(GL_TEXTURE_2D, *texturePtr);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  // Generate mipmaps
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glGenerateMipmap(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_2D, 0); // unbind
  return true;
}

bool loadDDSTexture(char const* fname, GLuint* texturePtr) {
  *texturePtr = 0;

  int fd = open(fname, O_RDONLY, 0);
  if(fd < 0) {
    std::cout << "ERROR: loadDDSTexture failed, fname =  " << fname << ", fd = " << fd << std::endl;
    return false;
  }
  defer(close(fd));

  struct stat st;
  if(fstat(fd, &st) < 0) {
    std::cout << "ERROR: loadDDSTexture failed, fstat < 0" << std::endl;
    return false;
  }
  size_t fsize = (size_t)st.st_size;

  void* dataPtr = mmap(nullptr, fsize, PROT_READ, MAP_PRIVATE | MAP_POPULATE, fd, 0);
  if(dataPtr == MAP_FAILED) {
    std::cout << "ERROR: loadDDSTexture() failed, mmap returned MAP_FAILED, " << strerror(errno) << std::endl;
    return false;
  }
  defer(munmap(dataPtr, fsize));

  std::cout << "loadDDSTexture() - Success, fsize = " << fsize << std::endl;

  // TODO parse DDS header


  glGenTextures(1, texturePtr);
  glBindTexture(GL_TEXTURE_2D, *texturePtr);


  glBindTexture(GL_TEXTURE_2D, 0); // unbind
  return true;
}

bool checkShaderCompileStatus(GLuint obj) {
  GLint status;
  glGetShaderiv(obj, GL_COMPILE_STATUS, &status);
  if(status == GL_FALSE) {
    GLint length;
    glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &length);
    std::vector<char> log((unsigned long)length);
    glGetShaderInfoLog(obj, length, &length, &log[0]);
    std::cerr << &log[0];
    return true;
  }
  return false;
}

bool checkProgramLinkStatus(GLuint obj) {
  GLint status;
  glGetProgramiv(obj, GL_LINK_STATUS, &status);
  if(status == GL_FALSE) {
    GLint length;
    glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &length);
    std::vector<char> log((unsigned long) length);
    glGetProgramInfoLog(obj, length, &length, &log[0]);
    std::cerr << &log[0];
    return true;
  }
  return false;
}

GLuint loadShader(char const *fname, GLenum shaderType, bool *errorFlagPtr) {
  std::ifstream fileStream(fname);
  std::stringstream buffer;
  buffer << fileStream.rdbuf();
  std::string shaderSource(buffer.str());

  GLuint shaderId = glCreateShader(shaderType);
  const GLchar * const shaderSourceCStr = shaderSource.c_str();

  glShaderSource(shaderId, 1, &shaderSourceCStr, nullptr);
  glCompileShader(shaderId);

  *errorFlagPtr = checkShaderCompileStatus(shaderId);
  if(*errorFlagPtr) {
    glDeleteShader(shaderId);
    return 0;
  }

  return shaderId;
}

GLuint prepareProgram(const std::vector<GLuint>& shaders, bool *errorFlagPtr) {
  *errorFlagPtr = false;

  GLuint programId = glCreateProgram();
  for(auto it = shaders.cbegin(); it != shaders.cend(); ++it) {
    glAttachShader(programId, *it);
  }
  glLinkProgram(programId);

  *errorFlagPtr = checkProgramLinkStatus(programId);
  if(*errorFlagPtr) {
    glDeleteProgram(programId);
    return 0;
  }

  return programId;
}