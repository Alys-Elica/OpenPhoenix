cmake_minimum_required(VERSION 3.25)

set(PROJECT_NAME ofnx)

project(${PROJECT_NAME} VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

add_library(${PROJECT_NAME} SHARED
    ofnx/files/tst.h
    ofnx/files/tst.cpp
    ofnx/files/vr.h
    ofnx/files/vr.cpp

    ofnx/graphics/dct.h
    ofnx/graphics/dct.cpp
    ofnx/graphics/rendereropengl.h
    ofnx/graphics/rendereropengl.cpp
    
    ofnx/tools/datastream.h
    ofnx/tools/datastream.cpp

    ofnx/ofnx_globals.h
    ofnx/ofnxmanager.h
    ofnx/ofnxmanager.cpp

    glad/gl.h
    glad/gl.c
    KHR/khrplatform.h
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_compile_definitions(
    OFNX_EXPORTS
)

# SDL3
find_package(SDL3 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)

# OpenGL
find_package(OpenGL REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE ${OPENGL_LIBRARIES})
target_include_directories(${PROJECT_NAME} PRIVATE ${OPENGL_INCLUDE_DIR})

# GLM
find_package(glm CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm-header-only)
