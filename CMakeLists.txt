cmake_minimum_required(VERSION 2.8)
project(vao_and_shaders)

find_package(OpenGL REQUIRED)

add_subdirectory(glfw)
add_subdirectory(glxw)

include_directories(glxw/include)
include_directories(glfw/include)
include_directories(glm)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -Wextra -std=c++11")
SET(LIBRARIES glfw glxw ${GLFW_LIBRARIES} ${GLXW_LIBRARY} ${OPENGL_LIBRARY})

set(SOURCE_FILES main.cpp)
add_executable(vao_and_shaders ${SOURCE_FILES})

target_link_libraries(vao_and_shaders ${LIBRARIES})