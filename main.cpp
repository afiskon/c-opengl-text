#include <GLXW/glxw.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
//#include <glm/gtx/transform.hpp>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
//#include <GL/gl.h>


static const GLfloat g_vertex_buffer_data[] = {
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        0.0f,  1.0f, 0.0f,
};

void setWindowSizeCallback(GLFWwindow*, int width, int height) {
  glViewport(0, 0, width, height);
}

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
          "layout(location = 0) in vec3 vertexPosition_modelspace;\n"
          "void main(){\n"
          "  gl_Position.xyz = vertexPosition_modelspace;\n"
          "  gl_Position.w = 1.0;\n"
          "}";

  GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
  char const * vertexShaderSourcePtr = vertexShaderSource.c_str();
  glShaderSource(vertexShaderId, 1, &vertexShaderSourcePtr, nullptr);
  glCompileShader(vertexShaderId);

  *errorFlagPtr = checkShaderCompileStatus(vertexShaderId);
  if(*errorFlagPtr) return 0;

  std::string fragmentShaderSource = ""
          "#version 330 core\n"
          "out vec3 color;\n"
          "void main() { color = vec3(1,0,0); }\n";

  GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
  char const * fragmentShaderSourcePtr = fragmentShaderSource.c_str();
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

//  glDeleteShader(vertexShaderId);
//  glDeleteShader(fragmentShaderId);

  return programId;
}

void windowSizeCallback(GLFWwindow *, int width, int height) {
  glViewport(0, 0, width, height);
}

int main() {
  if(glfwInit() == GL_FALSE) {
    std::cerr << "Failed to initialize GLFW" << std::endl;
    return -1;
  }

  glfwDefaultWindowHints();

  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

  GLFWwindow* window = glfwCreateWindow(300, 300, "VAO and Shaders",
                                        nullptr, nullptr);
  if(window == nullptr) {
    std::cerr << "Failed to open GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);

  if(glxwInit()) {
    std::cerr << "Failed to init GLXW" << std::endl;
    glfwDestroyWindow(window);
    glfwTerminate();
    return 1;
  }

  glfwSwapInterval(1);

  glfwSetWindowSizeCallback(window, windowSizeCallback);

  glfwShowWindow(window);

  glEnable(GL_DEPTH_TEST | GL_DOUBLEBUFFER);
  glDepthFunc(GL_LESS);

  glClearColor(0, 0, 0, 1);

//  glm::mat4 m = glm::perspective(45.0f, 4.0f / 3.0f, 1.0f, 100.0f);
//  glMatrixMode(GL_PROJECTION);
//  glLoadMatrixf(glm::value_ptr(m));

//  GLuint VertexArrayID;
//  glGenVertexArrays(1, &VertexArrayID);
//  glBindVertexArray(VertexArrayID);

  bool errorFlag;
  GLuint programId = prepareProgram(&errorFlag);
  if(errorFlag) {
    glfwDestroyWindow(window);
    glfwTerminate();
    return -1;
  }

  GLuint vbo; // vertexbuffer
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);


  // vao and vbo handle
  GLuint vao;

  // generate and bind the vao
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);


  while(glfwWindowShouldClose(window) == GL_FALSE) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(programId);


    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(
            0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
    );

    // Draw the triangle !
    glDrawArrays(GL_TRIANGLES, 0, 3); // 3 indices starting at 0 -> 1 triangle

    glDisableVertexAttribArray(0);
    
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}