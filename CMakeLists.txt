cmake_minimum_required(VERSION 3.31.5)

project(gr-base VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

set(geometry
    src/core/geometry/materialInstance.cpp
    src/core/geometry/boneWeight.cpp
    src/core/geometry/batching.cpp
    src/core/geometry/material.cpp
    src/core/geometry/shaderData.cpp
    src/core/geometry/mesh.cpp
)

set(animation
    src/core/animation/animation.cpp
    src/core/animation/keyframe.cpp
    src/core/animation/bone.cpp
)

set(file
    src/core/file/memoryStream.cpp
    src/core/file/fileStream.cpp
    src/core/file/file.cpp
)

set(thread
    src/core/thread/taskmanager.cpp
    src/core/thread/threadpool.cpp
)

set(core
    src/core/mainthreaddispatcher.cpp
    src/core/assetReference.cpp
    src/core/renderSystem.cpp
    src/core/rendertarget.cpp
    src/core/renderer.cpp
    src/core/nameManager.cpp
    src/core/gameObject.cpp
    src/core/debugdraw.cpp
    src/core/display.cpp
    src/core/console.cpp
    src/core/window.cpp
    src/core/string.cpp
    src/core/status.cpp
    src/core/skybox.cpp
    src/core/clock.cpp
    src/core/scene.cpp
    src/core/input.cpp
    src/core/chunk.cpp
)

set(binary
    src/core/binary/gHeader.cpp
)

set(script
    src/script/characterController.cpp
    src/script/skinnedMeshRender.cpp
    src/script/LODComponent.cpp
    src/script/meshCollider.cpp
    src/script/boxCollider.cpp
    src/script/meshFilter.cpp
    src/script/baserender.cpp
    src/script/meshRender.cpp
    src/script/transform.cpp
    src/script/rigidbody.cpp
    src/script/collider.cpp
    src/script/camera.cpp
    src/script/light.cpp
)

set(physics
    src/physics/physicsScene.cpp
)

set(culling
    src/core/culling/quadtree.cpp
    src/core/culling/frustum.cpp
    src/core/culling/octree.cpp
)

set(task
    src/core/task/runtime/gameobjectsavetask.cpp
    src/core/task/runtime/gameobjectloadtask.cpp

    src/core/task/runtime/animationloadtask.cpp
    src/core/task/runtime/animationsavetask.cpp
    src/core/task/animationmanager.cpp
    
    src/core/task/runtime/scenesavetask.cpp
    src/core/task/runtime/sceneloadtask.cpp
    src/core/task/runtime/textureloadtask.cpp

    src/core/task/runtime/shaderloadtask.cpp

    src/core/task/scenetaskmanager.cpp
    src/core/task/texturemanager.cpp
)

set(managers_src
    src/managers/gameobjectmanager.cpp
    src/managers/batchingmanager.cpp
    src/managers/materialmanager.cpp
    src/managers/eventManager.cpp
    src/managers/bvhtreemanager.cpp
    src/managers/rendermanager.cpp
    src/managers/octreemanager.cpp
    src/managers/assetsManager.cpp
    src/managers/shadermanager.cpp
    src/managers/sceneManager.cpp
    src/managers/worldmanager.cpp
    src/managers/meshmanager.cpp
)

set(ECS_SRC
    src/ecs/behaviorRegistry.cpp
    src/ecs/behaviorStorage.cpp
    src/ecs/behavior.cpp
    src/ecs/entity.cpp
)

set(EXTERNAL_SRC
    src/externals/array.c
)

set(ASSET_SRC
    src/asset/materialasset.cpp
    src/asset/modelasset.cpp
    src/asset/meshasset.cpp
)

SET(TASK_SRC
    src/task/shaderbuildtask.cpp
    src/task/modelloadtask.cpp
)

find_package(Bullet QUIET)
if(NOT Bullet_FOUND)
    message(FATAL_ERROR "Please install libbullet-dev")
    return()
endif()

add_library(${PROJECT_NAME} SHARED ${script} ${geometry} ${file} ${core} ${culling} ${task} ${binary} ${animation} ${physics} ${thread} ${managers_src} ${EXTERNAL_SRC} ${ECS_SRC} ${ASSET_SRC} ${TASK_SRC})

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    ${BULLET_INCLUDE_DIRS}
    imgui/src/backends
    imgui/src
)

target_link_libraries(${PROJECT_NAME} PRIVATE ${BULLET_LIBRARIES})

target_compile_definitions(${PROJECT_NAME} PUBLIC INTERNAL_REGISTER_SCRIPT)

