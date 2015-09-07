#include "utils.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <sstream>
#include <fstream>

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

GLuint loadShader(const char* fileName, GLenum shaderType, bool *errorFlagPtr) {
  std::ifstream fileStream(fileName);
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