cmake_minimum_required(VERSION 3.5.0)

project(gr-base VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

set(ecs
    src/ecs/componentRegistry.cpp
    src/ecs/gameObject.cpp
    src/ecs/component.cpp
    src/ecs/manager.cpp
    src/ecs/entity.cpp
    src/ecs/field.cpp
    src/ecs/pool.cpp
)

set(geometry
    src/core/geometry/boneWeight.cpp
    src/core/geometry/vertex2D.cpp
    src/core/geometry/vertex3D.cpp
    src/core/geometry/mesh.cpp
)

set(animation
    src/core/animation/animation.cpp
    src/core/animation/keyframe.cpp
    src/core/animation/bone.cpp
)

set(file
    src/core/file/stream.cpp
)

set(core
    src/core/nameManager.cpp
    src/core/threadpool.cpp
    src/core/display.cpp
    src/core/string.cpp
    src/core/event.cpp
)

set(script
    src/script/transform.cpp
    src/script/camera.cpp
)

add_library(${PROJECT_NAME} SHARED ${ecs} ${script} ${geometry} ${file} ${core} ${animation})

target_include_directories(${PROJECT_NAME} PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}>
)

target_compile_definitions(${PROJECT_NAME} PUBLIC INTERNAL_REGISTER_SCRIPT)

