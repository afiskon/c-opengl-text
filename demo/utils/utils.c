#include "utils.h"
#include "filemapping.h"

#include <stdio.h>
#include <stdlib.h>

#define DDS_HEADER_SIZE 128
#define DDS_SIGNATURE    0x20534444 // "DDS "
#define FORMAT_CODE_DXT1 0x31545844 // "DXT1"
#define FORMAT_CODE_DXT3 0x33545844 // "DXT3"
#define FORMAT_CODE_DXT5 0x35545844 // "DXT5"

// these used to be defined in glfw/deps/GL/glext.h
// until GLFW commit 1b1ef31228412cc0509240a52ac181b863bba87a
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT 0x8C4D
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT 0x8C4E
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT 0x8C4F

#ifdef _WIN32

#include <windows.h>

uint64_t
getCurrentTimeMs()
{
    FILETIME filetime;
    GetSystemTimeAsFileTime(&filetime);

    uint64_t nowWindows = (uint64_t)filetime.dwLowDateTime
        + ((uint64_t)(filetime.dwHighDateTime) << 32ULL);

    uint64_t nowUnix = nowWindows - 116444736000000000ULL;

    return nowUnix / 10000ULL;
}

#else // Linux, MacOS, etc

#include <sys/time.h>

uint64_t
getCurrentTimeMs()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return ((uint64_t)tv.tv_sec) * 1000 + ((uint64_t)tv.tv_usec) / 1000;
}
#endif

static bool
loadDDSTextureCommon(const char* fname, GLuint textureId,
                     unsigned int fsize, unsigned char* dataPtr)
{
    if(fsize < DDS_HEADER_SIZE)
    {
        fprintf(stderr, "loadDDSTexture failed, fname = %s, "
                    "fsize = %u, less then"
                    " DDS_HEADER_SIZE ( %d )\n",
                fname, fsize,  DDS_HEADER_SIZE);
        return false;
    }

    unsigned int signature    = *(unsigned int*)&(dataPtr[ 0]);
    unsigned int height       = *(unsigned int*)&(dataPtr[12]);
    unsigned int width        = *(unsigned int*)&(dataPtr[16]);
    unsigned int mipMapNumber = *(unsigned int*)&(dataPtr[28]);
    unsigned int formatCode   = *(unsigned int*)&(dataPtr[84]);

    if(signature != DDS_SIGNATURE)
    {
        fprintf(stderr, "loadDDSTexture failed, fname = %s,"
            " invalid signature: "
            "0x%08X\n", fname, signature);
        return false;
    }

    unsigned int format;
    switch(formatCode)
    {
        case FORMAT_CODE_DXT1:
            format = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT;
            break;
        case FORMAT_CODE_DXT3:
            format = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT;
            break;
        case FORMAT_CODE_DXT5:
            format = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
            break;
        default:
            fprintf(stderr, "loadDDSTexture failed, fname = %s,"
                " unknown formatCode:"
                " 0x%08X\n", fname, formatCode);
            return false;
    }

    glBindTexture(GL_TEXTURE_2D, textureId);

    unsigned int blockSize = (format ==
        GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT) ? 8 : 16;
    unsigned int offset = DDS_HEADER_SIZE;

    // load mipmaps
    for (unsigned int level = 0; level < mipMapNumber; ++level)
    {
        unsigned int size = ((width+3)/4)*((height+3)/4)*blockSize;
        if(fsize < offset + size) {
            fprintf(stderr, "loadDDSTexture failed, fname = %s,"
                        " fsize = %u, level ="
                        " %u, offset = %u, size = %u\n",
                    fname, fsize, level, offset, size);
            return false;
        }
        glCompressedTexImage2D(GL_TEXTURE_2D,
            level, format, width, height, 0, size, dataPtr + offset);

        width = width > 1 ? width >> 1 : 1;
        height = height > 1 ? height >> 1 : 1;
        offset += size;
    }

    glTexParameteri(GL_TEXTURE_2D,
        GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,
        GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    return true;
}

bool
loadDDSTexture(const char *fname, GLuint textureId)
{
    FileMapping* mapping = fileMappingCreate(fname);
    if(mapping == NULL) return false;

    unsigned char* dataPtr = fileMappingGetPointer(mapping);
    unsigned int fsize = fileMappingGetSize(mapping);

    bool res = loadDDSTextureCommon(fname, textureId, fsize, dataPtr);

    fileMappingDestroy(mapping);

    return res;
}

void
loadOneColorTexture(GLfloat r, GLfloat g, GLfloat b, GLuint textureId)
{
    glBindTexture(GL_TEXTURE_2D, textureId);
    unsigned char textureData[3];
    textureData[0] = (unsigned char)(r*255.0f);
    textureData[1] = (unsigned char)(g*255.0f);
    textureData[2] = (unsigned char)(b*255.0f);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0,
        GL_RGB, GL_UNSIGNED_BYTE, textureData);
}

static bool
checkShaderCompileStatus(GLuint obj)
{
    GLint status;
    glGetShaderiv(obj, GL_COMPILE_STATUS, &status);
    if(status == GL_FALSE) {
        GLint buffsize;
        glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &buffsize);

        char* buff = (char*)malloc((size_t) buffsize);
        if(!buff) {
            fprintf(stderr, "checkShaderCompileStatus - error!\n");
        } else {
            glGetShaderInfoLog(obj, buffsize, &buffsize, buff);
            fprintf(stderr, "checkShaderCompileStatus - error: %s\n", buff);
            free(buff);
        }
        return true;
    }
    return false;
}

static bool
checkProgramLinkStatus(GLuint obj)
{
    GLint status;
    glGetProgramiv(obj, GL_LINK_STATUS, &status);
    if(status == GL_FALSE) {
        GLint buffsize;
        glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &buffsize);

        char* buff = (char*)malloc((size_t) buffsize);
        if(!buff) {
            fprintf(stderr, "checkProgramLinkStatus - error!\n");
        } else {
            glGetProgramInfoLog(obj, buffsize, &buffsize, buff);
            fprintf(stderr, "checkProgramLinkStatus - error: %s\n", buff);
            free(buff);
        }
        return true;
    }
    return false;
}

GLuint
loadShader(const char *fname, GLenum shaderType, bool *errorFlagPtr)
{
    *errorFlagPtr = false;

    FileMapping* mapping = fileMappingCreate(fname);
    if(mapping == NULL) {
        *errorFlagPtr = true;
        return 0;
    }

    GLuint shaderId = glCreateShader(shaderType);
    GLchar* stringArray[1];
    GLint lengthArray[1];

    stringArray[0] = (GLchar*)fileMappingGetPointer(mapping);
    lengthArray[0] = fileMappingGetSize(mapping);

    glShaderSource(shaderId, 1,
        (GLchar const * const *)stringArray, lengthArray);
    glCompileShader(shaderId);

    fileMappingDestroy(mapping);

    *errorFlagPtr = checkShaderCompileStatus(shaderId);
    if(*errorFlagPtr) {
        glDeleteShader(shaderId);
        return 0;
    }

    return shaderId;
}

GLuint
prepareProgram(const GLuint* shaders, int nshaders, bool *errorFlagPtr)
{
    *errorFlagPtr = false;

    GLuint programId = glCreateProgram();

    for(int i = 0; i < nshaders; i++)
        glAttachShader(programId, shaders[i]);

    glLinkProgram(programId);

    *errorFlagPtr = checkProgramLinkStatus(programId);
    if(*errorFlagPtr) {
        glDeleteProgram(programId);
        return 0;
    }

    return programId;
}

GLint
getUniformLocation(GLuint programId, const char* uniformName)
{
    GLint location = glGetUniformLocation(programId, uniformName);
    if(location == -1) {
        fprintf(stderr, "getUniformLocation failed, programId = %u, "
                            "uniformName = %s\n",
                programId, uniformName);
    }
    return location;
}

void
setUniform1f(GLuint programId, const char* uniformName, float value)
{
    GLint uniformId = getUniformLocation(programId, uniformName);
    glUniform1f(uniformId, value);
}

void
setUniform3f(GLuint programId, const char* uniformName,
    float v1, float v2, float v3)
{
    GLint uniformId = getUniformLocation(programId, uniformName);
    glUniform3f(uniformId, v1, v2, v3);
}
