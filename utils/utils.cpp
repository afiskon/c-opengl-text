#include "utils.h"

// <GL/glext.h> on Linux, <OpenGL/glext.h> on MacOS :(
#include "../glfw/deps/GL/glext.h"

#include "fileMapping.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <defer.h>

#define DDS_HEADER_SIZE 128
#define DDS_SIGNATURE    0x20534444 // "DDS "
#define FORMAT_CODE_DXT1 0x31545844 // "DXT1"
#define FORMAT_CODE_DXT3 0x33545844 // "DXT3"
#define FORMAT_CODE_DXT5 0x35545844 // "DXT5"

static bool
loadDDSTextureCommon(const char* fname, GLuint textureId,
					 unsigned int fsize, unsigned char* dataPtr) {
  if(fsize < DDS_HEADER_SIZE) {
    std::cerr << "loadDDSTexture failed, fname = " << fname <<
      ", fsize = " << fsize << ", less then DDS_HEADER_SIZE (" << DDS_HEADER_SIZE << ")" << std::endl;
    return false;
  }

  unsigned int signature    = *(unsigned int*)&(dataPtr[ 0]);
  unsigned int height       = *(unsigned int*)&(dataPtr[12]);
  unsigned int width        = *(unsigned int*)&(dataPtr[16]);
  unsigned int mipMapNumber = *(unsigned int*)&(dataPtr[28]);
  unsigned int formatCode   = *(unsigned int*)&(dataPtr[84]);

  if(signature != DDS_SIGNATURE) {
    std::cerr << "loadDDSTexture failed, fname = " << fname <<
      "invalid signature: 0x" << std::hex << signature << std::endl;
    return false;
  }

  unsigned int format;
  switch(formatCode)
  {
    case FORMAT_CODE_DXT1:
      format = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT;
      break;
    case FORMAT_CODE_DXT3:
      format = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT;
      break;
    case FORMAT_CODE_DXT5:
      format = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
      break;
    default:
      std::cerr << "loadDDSTexture failed, fname = " << fname <<
        ", unknown formatCode: 0x" << std::hex << formatCode << std::endl;
      return false;
  }

  glBindTexture(GL_TEXTURE_2D, textureId);

  unsigned int blockSize = (format == GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT) ? 8 : 16;
  unsigned int offset = DDS_HEADER_SIZE;

  // load mipmaps
  for (unsigned int level = 0; level < mipMapNumber; ++level)
  {
    unsigned int size = ((width+3)/4)*((height+3)/4)*blockSize;
    if(fsize < offset + size) {
      std::cerr << "loadDDSTexture failed, fname = " << fname <<
        ", fsize = " << fsize << ", level = " << level <<
        ", offset = " << offset << ", size = " << size << std::endl;
      return false;
    }
    glCompressedTexImage2D(GL_TEXTURE_2D, level, format, width, height, 0, size, dataPtr + offset);

    width = width > 1 ? width >> 1 : 1;
    height = height > 1 ? height >> 1 : 1;
    offset += size;
  }

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

  return true;
}

bool loadDDSTexture(const char *fname, GLuint textureId) {
  FileMapping* mapping = fileMappingCreate(fname);
  if(mapping == nullptr) return false;
  defer(fileMappingClose(mapping));

  unsigned char* dataPtr = fileMappingGetPointer(mapping);
  unsigned int fsize = fileMappingGetSize(mapping);

  return loadDDSTextureCommon(fname, textureId, fsize, dataPtr);
}

void loadOneColorTexture(GLfloat r, GLfloat g, GLfloat b, GLuint textureId) {
  glBindTexture(GL_TEXTURE_2D, textureId);
  unsigned char textureData[3];
  textureData[0] = (unsigned char)(r*255.0f);
  textureData[1] = (unsigned char)(g*255.0f);
  textureData[2] = (unsigned char)(b*255.0f);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);
}

static bool
checkShaderCompileStatus(GLuint obj) {
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

static bool
checkProgramLinkStatus(GLuint obj) {
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

GLuint loadShader(const char *fname, GLenum shaderType, bool *errorFlagPtr) {
  FileMapping* mapping = fileMappingCreate(fname);
  if(mapping == nullptr) {
    *errorFlagPtr = true;
    return 0;
  }
  defer(fileMappingClose(mapping));

  GLuint shaderId = glCreateShader(shaderType);
  GLchar* stringArray[1];
  GLint lengthArray[1];

  stringArray[0] = (GLchar*)fileMappingGetPointer(mapping);
  lengthArray[0] = fileMappingGetSize(mapping);

  glShaderSource(shaderId, 1, (GLchar const * const *)stringArray, lengthArray);
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

GLint getUniformLocation(GLuint programId, const char* uniformName) {
  GLint location = glGetUniformLocation(programId, uniformName);
  if(location == -1) {
    std::cerr << "getUniformLocation failed, programId = " << programId << ", uniformName = " << uniformName;
  }
  return location;
}

void setUniform1f(GLuint programId, const char* uniformName, float value) {
  GLint uniformId = getUniformLocation(programId, uniformName);
  glUniform1f(uniformId, value);
}

void setUniform3f(GLuint programId, const char* uniformName, float v1, float v2, float v3) {
  GLint uniformId = getUniformLocation(programId, uniformName);
  glUniform3f(uniformId, v1, v2, v3);
}