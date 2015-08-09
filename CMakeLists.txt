cmake_minimum_required(VERSION 2.8)
project(vao_and_shaders)

find_package(OpenGL REQUIRED)

add_subdirectory(glfw)
add_subdirectory(glew)

include_directories(glew/include)
include_directories(glfw/include)
include_directories(glm)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -Wextra -std=c++11")

set(SOURCE_FILES main.cpp)
add_executable(vao_and_shaders ${SOURCE_FILES})

target_link_libraries(vao_and_shaders
  glfw libglew_static ${GLFW_LIBRARIES} ${GLEW_LIBRARIES} ${OPENGL_LIBRARY})