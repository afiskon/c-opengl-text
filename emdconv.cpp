#include <GLXW/glxw.h>

#include <vector>
#include "../assimp/include/assimp/Importer.hpp"
#include "../assimp/include/assimp/postprocess.h"
#include "../assimp/include/assimp/scene.h"
#include "utils/models.h"

// 3 per position + 3 per normal + UV
static const unsigned int floatsPerVertex = (3 + 3 + 2); 

GLfloat*
importedModelCreate(const char* fname, unsigned int meshNumber,
    size_t* outVerticesBufferSize, unsigned int* outVerticesNumber)
{
    *outVerticesBufferSize = 0;
    *outVerticesNumber = 0;
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(
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
        return NULL;
    }

    aiMesh* mesh = scene->mMeshes[meshNumber];
    unsigned int facesNum = mesh->mNumFaces;
    // unsigned int verticesNum = mesh->mNumVertices;

    if(mesh->mTextureCoords[0] == NULL)
    {
        fprintf(stderr,
                "mesh->mTextureCoords[0] == NULL, fname = %s\n",
                fname
            );
        return NULL;
    }

    unsigned int verticesPerFace = 3;
    *outVerticesNumber = facesNum*verticesPerFace;
    *outVerticesBufferSize = *outVerticesNumber * sizeof(GLfloat)
                                * floatsPerVertex;
    GLfloat* verticesBuffer = (GLfloat*)malloc(*outVerticesBufferSize);

    unsigned int verticesBufferIndex = 0;

    for(unsigned int i = 0; i < facesNum; ++i)
    {
        const aiFace& face = mesh->mFaces[i];
        if(face.mNumIndices != verticesPerFace)
        {
            fprintf(stderr,
                    "face.numIndices = %d (3 expected), i = %u, fname = %s\n",
                    face.mNumIndices, i, fname
                );
            free(verticesBuffer);
            return NULL;
        }

        for(unsigned int j = 0; j < face.mNumIndices; ++j)
        {
            unsigned int index = face.mIndices[j];
            aiVector3D pos = mesh->mVertices[index];
            aiVector3D uv = mesh->mTextureCoords[0][index];
            aiVector3D normal = mesh->mNormals[index];
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

    return verticesBuffer;
}

bool
importedModelSave(const char* fname, GLfloat* verticesBuffer,
    unsigned int verticesNumber)
{
    std::vector<GLfloat> vertices;
    std::vector<unsigned int> indices;
    unsigned int usedIndices = 0;
    
     // real case: 1.0f and 0.999969f should be considered equal
    const GLfloat eps = 0.00005f;

    for(unsigned int vtx = 0; vtx < verticesNumber; ++vtx)
    {
        unsigned int foundIndex = 0;
        bool indexFound = false;
        for(unsigned int idx = 0; !indexFound && idx < usedIndices; ++idx)
        {
            indexFound = true;
            // compare X, Y, Z, NX, NY, NZ, U and V
            for(unsigned int k = 0; indexFound && k < floatsPerVertex; ++k)
            {
                if(fabs(verticesBuffer[vtx * floatsPerVertex + k] -
                    vertices[idx * floatsPerVertex + k]) > eps)
                {
                    indexFound = false;
                }
            }

            if(indexFound) foundIndex = idx;
        }

        if(!indexFound)
        {
            // save X, Y, Z, NX, NY, NZ, U and V
            for(unsigned int k = 0; k < floatsPerVertex; ++k)
                vertices.push_back(verticesBuffer[vtx * floatsPerVertex + k]);
    
            foundIndex = usedIndices;
            usedIndices++;
        }

        indices.push_back(foundIndex);
    }

    unsigned char indexSize = 1;
    if(verticesNumber > 255) indexSize *= 2;
    if(verticesNumber > 65535) indexSize *= 2;

    unsigned int modelSize = (unsigned int) (
                                verticesNumber*floatsPerVertex*sizeof(GLfloat)
                            );
    unsigned int indexedModelSize = (unsigned int) (
                                usedIndices*floatsPerVertex*sizeof(GLfloat)
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

    return modelSave(
            fname,
            vertices.data(),
            usedIndices * floatsPerVertex * sizeof(GLfloat),
            indices.data(),
            verticesNumber
        );
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