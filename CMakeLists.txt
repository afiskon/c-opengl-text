cmake_minimum_required(VERSION 2.8)
project(opengl-demo)

find_package(OpenGL REQUIRED)

add_subdirectory(glfw)
add_subdirectory(glxw)
add_subdirectory(assimp)

include_directories(${OPENGL_INCLUDE_DIR})
include_directories(glxw/include)
include_directories(glfw/include)
include_directories(glm)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -Wextra -Werror")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -Werror -Wno-deprecated-register -Wno-shift-op-parentheses")

SET(MAIN_LIBRARIES glfw glxw ${GLFW_LIBRARIES} ${GLXW_LIBRARY}
                    ${OPENGL_LIBRARY} ${CMAKE_DL_LIBS})
set(MAIN_SOURCE_FILES demo/utils/camera.cpp demo/utils/camera.h 
                    demo/utils/linearalg.c demo/utils/linearalg.h
                    demo/utils/utils.cpp demo/utils/utils.h 
                    demo/utils/models.cpp demo/utils/models.h 
                    demo/utils/fileMapping.cpp demo/utils/fileMapping.h)
add_executable(demo demo/main.cpp ${MAIN_SOURCE_FILES})
target_link_libraries(demo ${MAIN_LIBRARIES})

SET(EMDCONV_LIBRARIES glfw glxw assimp ${GLFW_LIBRARIES} ${GLXW_LIBRARY} 
                        ${CMAKE_DL_LIBS})
set(EMDCONV_SOURCE_FILES demo/utils/models.cpp demo/utils/models.h 
                        demo/utils/fileMapping.cpp demo/utils/fileMapping.h)
add_executable(emdconv demo/emdconv.cpp ${EMDCONV_SOURCE_FILES})
target_link_libraries(emdconv ${EMDCONV_LIBRARIES})
