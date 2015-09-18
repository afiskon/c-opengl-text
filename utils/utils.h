#ifndef AFISKON_OPENGL_UTILS_H
#define AFISKON_OPENGL_UTILS_H

#include <GLXW/glxw.h>
#include <vector>

bool loadCommonTexture(const char* fname, GLuint textureId);
bool loadCommonTextureExt(const char* fname, GLuint textureId, bool flip);

bool loadDDSTexture(char const *fname, GLuint textureId);

GLuint prepareProgram(const std::vector<GLuint>& shaders, bool *errorFlagPtr);
GLuint loadShader(char const* fname, GLenum shaderType, bool * errorFlagPtr);

#endif // AFISKON_OPENGL_UTILS_H
