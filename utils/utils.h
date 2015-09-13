#ifndef TRIANGLE_UTILS_H
#define TRIANGLE_UTILS_H

#include <GLXW/glxw.h>
#include <vector>

void flipTexture(unsigned char *textureData, int width, int height, int n);

GLuint prepareProgram(const std::vector<GLuint>& shaders, bool *errorFlagPtr);
GLuint loadShader(const char* fileName, GLenum shaderType, bool *errorFlagPtr);

#endif //TRIANGLE_UTILS_H
