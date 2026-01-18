cmake_minimum_required(VERSION 3.25)

set(PROJECT_NAME ofnx)

project(${PROJECT_NAME} VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)

add_library(${PROJECT_NAME} SHARED
    src/ofnx/files/4xm.cpp
    src/ofnx/files/arnvit.cpp
    src/ofnx/files/lst.cpp
    src/ofnx/files/pak.cpp
    src/ofnx/files/tst.cpp
    src/ofnx/files/vr.cpp

    src/ofnx/graphics/dct.cpp
    src/ofnx/graphics/rendereropengl.cpp

    src/ofnx/tools/datastream.cpp

    src/glad/gl.c
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_compile_definitions(
    OFNX_EXPORTS
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# OpenGL
find_package(OpenGL REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE ${OPENGL_LIBRARIES})
target_include_directories(${PROJECT_NAME} PRIVATE ${OPENGL_INCLUDE_DIR})

# GLM
find_package(glm CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)
