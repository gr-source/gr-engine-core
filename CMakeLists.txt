cmake_minimum_required(VERSION 3.31.5)

project(gr-base VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

set(ecs
    src/ecs/componentRegistry.cpp
    src/ecs/scriptpriority.cpp
    src/ecs/chunkManager.cpp
    src/ecs/gameObject.cpp
    src/ecs/component.cpp
    src/ecs/manager.cpp
    src/ecs/entity.cpp
    src/ecs/field.cpp
    src/ecs/pool.cpp
)

set(geometry
    src/core/geometry/batchingRender.cpp
    src/core/geometry/boneWeight.cpp
    src/core/geometry/vertex2D.cpp
    src/core/geometry/vertex3D.cpp
    src/core/geometry/batching.cpp
    src/core/geometry/material.cpp
    src/core/geometry/boundbox.cpp
    src/core/geometry/shader.cpp
    src/core/geometry/mesh.cpp
)

set(animation
    src/core/animation/animation.cpp
    src/core/animation/keyframe.cpp
    src/core/animation/bone.cpp
)

set(file
    src/core/file/stream.cpp
    src/core/file/file.cpp
)

set(core
    src/core/assetsManager.cpp
    src/core/rendertarget.cpp
    src/core/nameManager.cpp
    src/core/threadpool.cpp
    src/core/display.cpp
    src/core/console.cpp
    src/core/window.cpp
    src/core/string.cpp
    src/core/status.cpp
    src/core/skybox.cpp
    src/core/scene.cpp
    src/core/event.cpp
    src/core/clock.cpp
    src/core/input.cpp
)

set(filmbox
    src/core/filmbox/fbxAnimation.cpp
    src/core/filmbox/fbxMaterial.cpp
    src/core/filmbox/fbxChannel.cpp
    src/core/filmbox/fbxVertex.cpp
    src/core/filmbox/fbxScene.cpp
    src/core/filmbox/fbxBone.cpp
    src/core/filmbox/fbxMesh.cpp
    src/core/filmbox/fbxNode.cpp
    src/core/filmbox/filmbox.cpp
)

set(binary
    src/core/binary/modelhandle.cpp
    src/core/binary/gHeader.cpp
)

set(script
    src/script/skinnedMeshRender.cpp
    src/script/meshCollider.cpp
    src/script/boxCollider.cpp
    src/script/meshFilter.cpp
    src/script/baserender.cpp
    src/script/meshRender.cpp
    src/script/rigidbody.cpp
    src/script/transform.cpp
    src/script/animator.cpp
    src/script/camera.cpp
    src/script/light.cpp
)

set(physics
    src/physics/physicsScene.cpp
)

set(culling
    src/core/culling/frustum.cpp
)

set(task
    src/core/task/gameobjectsavetask.cpp
    src/core/task/gameobjectloadtask.cpp

    src/core/task/animationloadtask.cpp
    src/core/task/animationsavetask.cpp
    src/core/task/animationmanager.cpp
    
    src/core/task/shaderProperty.cpp
    src/core/task/scenesavetask.cpp
    src/core/task/sceneloadtask.cpp
    src/core/task/textureloadtask.cpp

    src/core/task/shaderloadtask.cpp
    src/core/task/shadermanager.cpp

    src/core/task/filmboxmanager.cpp
    src/core/task/filmboxtask.cpp
)

find_package(assimp QUIET)
if(NOT assimp_FOUND)
    message(FATAL_ERROR "Please install libassimp-dev")
    return()
endif()

find_package(Bullet QUIET)
if(NOT Bullet_FOUND)
    message(FATAL_ERROR "Please install libbullet-dev")
    return()
endif()

add_library(${PROJECT_NAME} SHARED ${ecs} ${script} ${geometry} ${file} ${core} ${culling} ${filmbox} ${task} ${binary} ${animation} ${physics})

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    ${BULLET_INCLUDE_DIRS}
    imgui/src/backends
    imgui/src
)

target_link_libraries(${PROJECT_NAME} PRIVATE assimp ${BULLET_LIBRARIES})

target_compile_definitions(${PROJECT_NAME} PUBLIC INTERNAL_REGISTER_SCRIPT)

