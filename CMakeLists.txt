cmake_minimum_required(VERSION 2.8)
project(opengl-demo)

find_package(OpenGL REQUIRED)

add_subdirectory(glfw)
add_subdirectory(glxw)
add_subdirectory(assimp)

include_directories(${OPENGL_INCLUDE_DIR})
include_directories(glxw/include)
include_directories(glfw/include)
include_directories(assimp/include)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -O2 -Wall -Wextra -Werror")

SET(MAIN_LIBRARIES glfw glxw ${GLFW_LIBRARIES} ${GLXW_LIBRARY}
                    ${OPENGL_LIBRARY} ${CMAKE_DL_LIBS})
set(MAIN_SOURCE_FILES demo/utils/camera.c demo/utils/camera.h 
                    demo/utils/linearalg.c demo/utils/linearalg.h
                    demo/utils/utils.c demo/utils/utils.h 
                    demo/utils/models.c demo/utils/models.h 
                    demo/utils/filemapping.c demo/utils/filemapping.h)
add_executable(demo demo/main.c ${MAIN_SOURCE_FILES})
target_link_libraries(demo ${MAIN_LIBRARIES})

SET(EMDCONV_LIBRARIES glfw glxw assimp ${GLFW_LIBRARIES} ${GLXW_LIBRARY} 
                        ${CMAKE_DL_LIBS})
set(EMDCONV_SOURCE_FILES demo/utils/models.c demo/utils/models.h 
                        demo/utils/filemapping.c demo/utils/filemapping.h)
add_executable(emdconv demo/emdconv.c ${EMDCONV_SOURCE_FILES})
target_link_libraries(emdconv ${EMDCONV_LIBRARIES})
