cmake_minimum_required(VERSION 2.8)
project(lighting)

find_package(OpenGL REQUIRED)

add_subdirectory(glfw)
add_subdirectory(glxw)
add_subdirectory(assimp)

include_directories(${OPENGL_INCLUDE_DIR})
include_directories(glxw/include)
include_directories(glfw/include)
include_directories(glm)
include_directories(deferxx)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -std=c++11 -Wall -Wextra -Werror -Wno-deprecated-register -Wno-shift-op-parentheses")

SET(MAIN_LIBRARIES glfw glxw ${GLFW_LIBRARIES} ${GLXW_LIBRARY} ${OPENGL_LIBRARY})
set(MAIN_SOURCE_FILES utils/camera.cpp utils/camera.h utils/utils.h utils/utils.cpp utils/models.cpp utils/models.h utils/fileMapping.cpp utils/fileMapping.h)
add_executable(lighting main.cpp ${MAIN_SOURCE_FILES})
target_link_libraries(lighting ${MAIN_LIBRARIES})

SET(EMDCONV_LIBRARIES glfw glxw assimp ${GLFW_LIBRARIES} ${GLXW_LIBRARY})
set(EMDCONV_SOURCE_FILES emdconv.cpp utils/models.cpp utils/models.h utils/fileMapping.cpp utils/fileMapping.h)
add_executable(emdconv ${EMDCONV_SOURCE_FILES})
target_link_libraries(emdconv ${EMDCONV_LIBRARIES})