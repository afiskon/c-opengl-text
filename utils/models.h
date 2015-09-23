#ifndef AFISKON_MODELS_H
#define AFISKON_MODELS_H

#include <GL/gl.h>
#include <cstddef>

bool saveModel(const char* fname, const void* verticesData, size_t verticesDataSize, const void* indicesData, size_t indicesDataSize);
bool loadModel(const char* fname, GLuint modelVAO, GLuint modelVBO, GLuint indicesVBO);

// TODO: 1) LoadedModel struct 2) rename to modelSave, modelLoad 3) create modelRender

#endif // AFISKON_MODELS_H
