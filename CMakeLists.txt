cmake_minimum_required(VERSION 3.5.0)

project(gr-base VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

set(ecs_src
    src/ecs/componentRegistry.cpp
    src/ecs/gComponent.cpp
    src/ecs/gEntity.cpp
    src/ecs/field.cpp
    src/ecs/pool.cpp
)

add_library(${PROJECT_NAME} SHARED ${ecs_src} )

target_include_directories(${PROJECT_NAME} PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}>
)