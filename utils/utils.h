#ifndef AFISKON_OPENGL_UTILS_H
#define AFISKON_OPENGL_UTILS_H

#include <GLXW/glxw.h>
#include <vector>

bool loadCommonTexture(const char* fname, GLuint* texturePtr);
bool loadCommonTextureExt(const char* fname, GLuint* texturePtr, bool flip);

bool loadDDSTexture(char const *fname, GLuint *texturePtr);
bool loadDDSTextureExt(char const *fname, GLuint *texturePtr, bool flip);

GLuint prepareProgram(const std::vector<GLuint>& shaders, bool *errorFlagPtr);
GLuint loadShader(char const* fname, GLenum shaderType, bool * errorFlagPtr);

#endif // AFISKON_OPENGL_UTILS_H
