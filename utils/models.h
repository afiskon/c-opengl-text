#ifndef AFISKON_MODELS_H
#define AFISKON_MODELS_H

#include <GLXW/glxw.h>
#include <cstddef>

bool modelSave(const char *fname, const void *verticesData, size_t verticesDataSize, const void *indicesData, size_t indicesDataSize, unsigned char indexSize);
bool modelLoad(const char *fname, GLuint modelVAO, GLuint modelVBO, GLuint indicesVBO, GLsizei* outIndicesNumber, GLenum* outIndicesType);

// TODO: optimize + indices
GLfloat* importedModelCreate(const char* fname, unsigned int meshNumber, size_t* outVerticesBufferSize, unsigned int* outVerticesNumber);
bool importedModelSave(const char* fname, GLfloat* verticesBuffer, unsigned int verticesNumber);
void importedModelFree(GLfloat* model);

#endif // AFISKON_MODELS_H
