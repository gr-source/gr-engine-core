cmake_minimum_required(VERSION 3.5.0)

project(gr-base VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

set(ecs
    src/ecs/componentRegistry.cpp
    src/ecs/component.cpp
    src/ecs/gEntity.cpp
    src/ecs/field.cpp
    src/ecs/pool.cpp
)

set(geometry
    src/core/geometry/boneWeight.cpp
    src/core/geometry/vertex2D.cpp
    src/core/geometry/vertex3D.cpp
    src/core/geometry/mesh.cpp
)

set(file
    src/core/file/stream.cpp
)

set(core
)

add_library(${PROJECT_NAME} SHARED ${ecs} ${geometry} ${file} ${core})

target_include_directories(${PROJECT_NAME} PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}>
)