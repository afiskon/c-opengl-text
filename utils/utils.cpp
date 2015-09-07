#include "utils.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>

// TODO: move shaders to seperate files

bool checkShaderCompileStatus(GLuint obj) {
    GLint status;
    glGetShaderiv(obj, GL_COMPILE_STATUS, &status);
    if(status == GL_FALSE) {
        GLint length;
        glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> log((unsigned long)length);
        glGetShaderInfoLog(obj, length, &length, &log[0]);
        std::cerr << &log[0];
        return true;
    }
    return false;
}

bool checkProgramLinkStatus(GLuint obj) {
    GLint status;
    glGetProgramiv(obj, GL_LINK_STATUS, &status);
    if(status == GL_FALSE) {
        GLint length;
        glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> log((unsigned long)length);
        glGetProgramInfoLog(obj, length, &length, &log[0]);
        std::cerr << &log[0];
        return true;
    }
    return false;
}

GLuint prepareProgram(bool *errorFlagPtr) {
    *errorFlagPtr = false;

    std::string vertexShaderSource = ""
            "#version 330 core\n"
            "layout(location = 0) in vec3 vertexPos;\n"
            "void main(){\n"
            "  gl_Position.xyz = vertexPos;\n"
            "  gl_Position.w = 1.0;\n"
            "}";

    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    const GLchar * const vertexShaderSourcePtr = vertexShaderSource.c_str();
    glShaderSource(vertexShaderId, 1, &vertexShaderSourcePtr, nullptr);
    glCompileShader(vertexShaderId);

    *errorFlagPtr = checkShaderCompileStatus(vertexShaderId);
    if(*errorFlagPtr) return 0;

    std::string fragmentShaderSource = ""
            "#version 330 core\n"
            "out vec3 color;\n"
            "void main() { color = vec3(0,0,1); }\n";

    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar * const fragmentShaderSourcePtr = fragmentShaderSource.c_str();
    glShaderSource(fragmentShaderId, 1, &fragmentShaderSourcePtr, nullptr);
    glCompileShader(fragmentShaderId);

    *errorFlagPtr = checkShaderCompileStatus(fragmentShaderId);
    if(*errorFlagPtr) return 0;

    GLuint programId = glCreateProgram();
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);
    glLinkProgram(programId);

    *errorFlagPtr = checkProgramLinkStatus(programId);
    if(*errorFlagPtr) return 0;

    glDeleteShader(vertexShaderId);
    glDeleteShader(fragmentShaderId);

    return programId;
}