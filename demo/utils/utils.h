#ifndef AFISKON_UTILS_H
#define AFISKON_UTILS_H

#include <GLXW/glxw.h>
#include <stdbool.h>

bool loadDDSTexture(const char *fname, GLuint textureId);
void loadOneColorTexture(GLfloat r, GLfloat g, GLfloat b, GLuint textureId);

GLuint loadShader(const char * fname, GLenum shaderType, bool * errorFlagPtr);
GLuint prepareProgram(const GLuint* shaders, int nshaders, bool *errorFlagPtr);
GLint getUniformLocation(GLuint programId, const char* uniformName);
void setUniform1f(GLuint programId, const char* uniformName, float value);
void setUniform3f(GLuint programId, const char* uniformName,
	float v1, float v2, float v3);

uint64_t getCurrentTimeMs();

#endif // AFISKON_UTILS_H
