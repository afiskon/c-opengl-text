#include <GLXW/glxw.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "../assimp/include/assimp/cimport.h"
#include "../assimp/include/assimp/postprocess.h"
#include "../assimp/include/assimp/scene.h"
#include "utils/models.h"

// 3 per position + 3 per normal + UV
#define FLOATS_PER_VERTEX (3 + 3 + 2)

// real case: 1.0f and 0.999969f should be considered equal
#define MODEL_FLOAT_EPS 0.00005f

// should be enough, actually only 18432 in torus.blend
#define MAX_VERTICES 65536

// should be enough, actually only 3456 in torus.blend
#define MAX_INDICES 16384

GLfloat*
importedModelCreate(const char* fname, unsigned int meshNumber,
    size_t* outVerticesBufferSize, unsigned int* outVerticesNumber)
{
    *outVerticesBufferSize = 0;
    *outVerticesNumber = 0;
    const struct aiScene* scene = aiImportFile(
            fname,
            aiProcess_CalcTangentSpace | aiProcess_Triangulate | 
            aiProcess_JoinIdenticalVertices | aiProcess_SortByPType
        );

    if(scene == NULL)
    {
        fprintf(stderr, "Failed to load model %s\n", fname);
        return NULL;
    }

    if(scene->mNumMeshes <= meshNumber)
    {
        fprintf(stderr,
                "There is no mesh #%u in model (%d only), fname = %s\n",
                meshNumber, scene->mNumMeshes, fname
            );
        aiReleaseImport(scene);
        return NULL;
    }

    struct aiMesh* mesh = scene->mMeshes[meshNumber];
    unsigned int facesNum = mesh->mNumFaces;
    // unsigned int verticesNum = mesh->mNumVertices;

    if(mesh->mTextureCoords[0] == NULL)
    {
        fprintf(stderr,
                "mesh->mTextureCoords[0] == NULL, fname = %s\n",
                fname
            );
        aiReleaseImport(scene);
        return NULL;
    }

    unsigned int verticesPerFace = 3;
    *outVerticesNumber = facesNum*verticesPerFace;
    *outVerticesBufferSize = *outVerticesNumber * sizeof(GLfloat)
                                * FLOATS_PER_VERTEX;
    GLfloat* verticesBuffer = (GLfloat*)malloc(*outVerticesBufferSize);

    unsigned int verticesBufferIndex = 0;

    for(unsigned int i = 0; i < facesNum; ++i)
    {
        if(mesh->mFaces[i].mNumIndices != verticesPerFace)
        {
            fprintf(stderr,
                    "mesh->mFaces[i].numIndices = %d (3 expected),"
                    " i = %u, fname = %s\n",
                    mesh->mFaces[i].mNumIndices, i, fname
                );
            free(verticesBuffer);
            aiReleaseImport(scene);
            return NULL;
        }

        for(unsigned int j = 0; j < mesh->mFaces[i].mNumIndices; ++j)
        {
            unsigned int index = mesh->mFaces[i].mIndices[j];
            struct aiVector3D pos = mesh->mVertices[index];
            struct aiVector3D uv = mesh->mTextureCoords[0][index];
            struct aiVector3D normal = mesh->mNormals[index];
            verticesBuffer[verticesBufferIndex++] = pos.x;
            verticesBuffer[verticesBufferIndex++] = pos.y;
            verticesBuffer[verticesBufferIndex++] = pos.z;
            verticesBuffer[verticesBufferIndex++] = normal.x;
            verticesBuffer[verticesBufferIndex++] = normal.y;
            verticesBuffer[verticesBufferIndex++] = normal.z;
            verticesBuffer[verticesBufferIndex++] = uv.x;
            verticesBuffer[verticesBufferIndex++] = 1.0f - uv.y;
        }
    }

    aiReleaseImport(scene);

    return verticesBuffer;
}

bool
importedModelSave(const char* fname, GLfloat* verticesBuffer,
    unsigned int verticesNumber)
{
    bool error = false;
    unsigned int verticesArrSize = 0;
    unsigned int indicesArrSize = 0;
    unsigned int usedIndices = 0;
    
    GLfloat* vertices = (GLfloat*)malloc(sizeof(GLfloat) * MAX_VERTICES);
    if(vertices == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for vertices\n");
        return false;
    }

    unsigned int* indices = (unsigned int*)malloc(
                                    sizeof(unsigned int) * MAX_INDICES
                                );
    if(indices == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for indices\n");
        free(vertices);
        return false;   
    }

    for(unsigned int vtx = 0; vtx < verticesNumber; ++vtx)
    {
        unsigned int foundIndex = 0;
        bool indexFound = false;
        for(unsigned int idx = 0; !indexFound && idx < usedIndices; ++idx)
        {
            indexFound = true;
            // compare X, Y, Z, NX, NY, NZ, U and V
            for(unsigned int k = 0; indexFound && k < FLOATS_PER_VERTEX; ++k)
            {
                if(fabs(verticesBuffer[vtx * FLOATS_PER_VERTEX + k] -
                    vertices[idx * FLOATS_PER_VERTEX + k]) > MODEL_FLOAT_EPS)
                {
                    indexFound = false;
                }
            }

            if(indexFound) foundIndex = idx;
        }

        if(!indexFound)
        {
            // save X, Y, Z, NX, NY, NZ, U and V
            for(unsigned int k = 0; k < FLOATS_PER_VERTEX; ++k)
            {
                if(verticesArrSize >= MAX_VERTICES)
                {
                    error = true;
                    break;
                }
                vertices[verticesArrSize++] = verticesBuffer[
                                                    vtx * FLOATS_PER_VERTEX + k
                                                ];
            }
    
            foundIndex = usedIndices;
            usedIndices++;
        }

        if(error || (indicesArrSize >= MAX_INDICES))
        {
            error = true;
            break;
        }

        indices[indicesArrSize++] = foundIndex;
    }

    if(error)
    {
        free(indices);
        free(vertices);
        return false;
    }

    assert(indicesArrSize == verticesNumber);

    unsigned char indexSize = 1;
    if(verticesNumber > 255) indexSize *= 2;
    if(verticesNumber > 65535) indexSize *= 2;

    unsigned int modelSize = (unsigned int) (
                                verticesNumber*FLOATS_PER_VERTEX*sizeof(GLfloat)
                            );
    unsigned int indexedModelSize = (unsigned int) (
                                usedIndices*FLOATS_PER_VERTEX*sizeof(GLfloat)
                                    + verticesNumber*indexSize
                            );
    float ratio = (float)indexedModelSize*100.0f / (float)modelSize;
    fprintf(stderr,
            "importedModelSave - fname = %s, verticesNumber = %u, "
            "usedIndices = %u\n", fname, verticesNumber, usedIndices
        );
    fprintf(stderr,
            "importedModelSave - modelSize = %u, indexedModelSize = %u, "
            "ratio = %f%%\n", modelSize, indexedModelSize, ratio
        );

    bool res = modelSave(
            fname,
            vertices,
            usedIndices * FLOATS_PER_VERTEX * sizeof(GLfloat),
            indices,
            verticesNumber
        );

    free(indices);
    free(vertices);
    return res;
}

void
importedModelFree(GLfloat* model)
{
    free(model);
}

int
main(int argc, char* argv[])
{
    if(argc < 2) {
        printf("Usage: emdconv <input file> <output file> [mesh number]\n");
        return 1;
    }

    char* infile = argv[1];
    char* outfile = argv[2];

    unsigned int meshNumber = 0;
    if(argc > 3) {
        meshNumber = (unsigned int) atoi(argv[3]);
    }

    printf("Infile: %s\n", infile);
    printf("Outfile: %s\n", outfile);
    printf("Mesh number: %u\n", meshNumber);

    unsigned int modelVerticesNumber;
    size_t modelVerticesBufferSize;
    GLfloat * modelVerticesBuffer = importedModelCreate(
                                            infile,
                                            meshNumber, 
                                            &modelVerticesBufferSize, 
                                            &modelVerticesNumber
                                        );
    if(modelVerticesBuffer == NULL)
    {
        fprintf(stderr, "importedModelCreate returned null\n");
        return 2;
    }

    if(!importedModelSave(outfile, modelVerticesBuffer, modelVerticesNumber))
    {
        fprintf(stderr, "importedModelSave failed\n");
        importedModelFree(modelVerticesBuffer);
        return 3;
    }

    printf("Done!\n");

    importedModelFree(modelVerticesBuffer);
    return 0;
}