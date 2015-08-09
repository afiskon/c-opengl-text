#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

#include <GLFW/glfw3.h>
#include <iostream>

void setWindowSizeCallback(GLFWwindow*, int width, int height) {
  glViewport(0, 0, width, height);
}

int main() {
  if(glfwInit() == GL_FALSE) {
    std::cerr << "Failed to initialize GLFW" << std::endl;
    return -1;
  }

  glfwDefaultWindowHints();

  GLFWwindow* window = glfwCreateWindow(300, 300, "VAO and Shaders",
                                        nullptr, nullptr);
  if(window == nullptr) {
    std::cerr << "Failed to open GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);

  if(glewInit() != GLEW_OK) {
    std::cerr << "Failed to initialize GLEW" << std::endl;
    return -1;
  }

  glfwSwapInterval(1);

  glfwSetWindowSizeCallback(window, setWindowSizeCallback);

  glfwShowWindow(window);

  glEnable(GL_DEPTH_TEST | GL_DOUBLEBUFFER);
  glDepthFunc(GL_LESS);

  glClearColor(0, 0, 0, 1);

  glm::mat4 m = glm::perspective(45.0f, 4.0f / 3.0f, 1.0f, 100.0f);
  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf(glm::value_ptr(m));

  GLuint VertexArrayID;
  glGenVertexArrays(1, &VertexArrayID);
  glBindVertexArray(VertexArrayID);

  while(glfwWindowShouldClose(window) == GL_FALSE) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /* */
    glColor4f(1, 0, 0, 1);
    glBegin(GL_TRIANGLES);
    glVertex3f( 0,  0.5, -5);
    glVertex3f( 0.5, -0.5, -5);
    glVertex3f(-0.5, -0.5, -5);
    glEnd();
    /* */

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}