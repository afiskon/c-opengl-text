cmake_minimum_required(VERSION 2.8)
project(rotating_cube)

find_package(OpenGL REQUIRED)

add_subdirectory(glfw)
add_subdirectory(glxw)

include_directories(glxw/include)
include_directories(glfw/include)
include_directories(glm)
include_directories(deferxx)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -Wextra -Werror -std=c++11")
SET(LIBRARIES glfw glxw ${GLFW_LIBRARIES} ${GLXW_LIBRARY} ${OPENGL_LIBRARY})

set(SOURCE_FILES main.cpp utils/camera.cpp utils/camera.h utils/utils.h utils/utils.cpp)
add_executable(rotating_cube ${SOURCE_FILES})

target_link_libraries(rotating_cube ${LIBRARIES})