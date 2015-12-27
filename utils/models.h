#ifndef AFISKON_MODELS_H
#define AFISKON_MODELS_H

#include <GLXW/glxw.h>

bool modelSave(const char *fname, const GLfloat *verticesData, size_t verticesDataSize, const unsigned int *indices, unsigned int indicesNumber);
bool modelLoad(const char *fname, GLuint modelVAO, GLuint modelVBO, GLuint indicesVBO, GLsizei* outIndicesNumber, GLenum* outIndicesType);

#endif // AFISKON_MODELS_H
