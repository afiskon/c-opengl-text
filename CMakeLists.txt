cmake_minimum_required(VERSION 2.8)
project(skybox)

find_package(OpenGL REQUIRED)

add_subdirectory(glfw)
add_subdirectory(glxw)

include_directories(${OPENGL_INCLUDE_DIR})
include_directories(glxw/include)
include_directories(glfw/include)
include_directories(glm)
include_directories(deferxx)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -std=c++11 -Wall -Wextra -Werror -Wno-deprecated-register -Wno-shift-op-parentheses")
SET(LIBRARIES glfw glxw ${GLFW_LIBRARIES} ${GLXW_LIBRARY} ${OPENGL_LIBRARY})

set(SOURCE_FILES main.cpp utils/camera.cpp utils/camera.h utils/utils.h utils/utils.cpp utils/models.cpp utils/models.h utils/fileMapping.cpp utils/fileMapping.h)
add_executable(skybox ${SOURCE_FILES})

target_link_libraries(skybox ${LIBRARIES})
