#ifndef AFISKON_OPENGL_UTILS_H
#define AFISKON_OPENGL_UTILS_H

#include <GLXW/glxw.h>
#include <vector>

bool loadDDSTexture(const char *fname, GLuint textureId);
void loadOneColorTexture(GLfloat r, GLfloat g, GLfloat b, GLuint textureId);

GLuint loadShader(const char * fname, GLenum shaderType, bool * errorFlagPtr);
GLuint prepareProgram(const std::vector<GLuint>& shaders, bool *errorFlagPtr);
GLint getUniformLocation(GLuint programId, const char* uniformName);
void setUniform1f(GLuint programId, const char* uniformName, float value);
void setUniform3f(GLuint programId, const char* uniformName, float v1, float v2, float v3);

long getCurrentTimeMs();

#endif // AFISKON_OPENGL_UTILS_H
