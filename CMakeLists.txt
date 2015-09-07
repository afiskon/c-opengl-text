cmake_minimum_required(VERSION 2.8)
project(triangle)

find_package(OpenGL REQUIRED)

add_subdirectory(glfw)
add_subdirectory(glxw)

include_directories(glxw/include)
include_directories(glfw/include)
include_directories(glm)
include_directories(deferxx)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -Wextra -Werror -std=c++11")
SET(LIBRARIES glfw glxw ${GLFW_LIBRARIES} ${GLXW_LIBRARY} ${OPENGL_LIBRARY})

set(SOURCE_FILES main.cpp)
add_executable(triangle ${SOURCE_FILES} utils/utils.h utils/utils.cpp)

target_link_libraries(triangle ${LIBRARIES})