#ifndef AFISKON_OPENGL_UTILS_H
#define AFISKON_OPENGL_UTILS_H

#include <GLXW/glxw.h>
#include <vector>

struct FileMapping;

FileMapping * fileMappingCreate(const char* fname);
unsigned char* fileMappingGetPointer(FileMapping * mapping);
unsigned int fileMappingGetSize(FileMapping * mapping);
void fileMappingClose(FileMapping * mapping);

bool loadDDSTexture(const char *fname, GLuint textureId);

GLuint prepareProgram(const std::vector<GLuint>& shaders, bool *errorFlagPtr);
GLuint loadShader(const char * fname, GLenum shaderType, bool * errorFlagPtr);

#endif // AFISKON_OPENGL_UTILS_H
