cmake_minimum_required(VERSION 3.31.5)

project(gr-base VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

set(geometry
    src/core/geometry/boneWeight.cpp
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
    src/core/assetDB.cpp
    src/core/rendertarget.cpp
    src/core/renderer.cpp
    src/core/gameObject.cpp
    src/core/debugdraw.cpp
    src/core/display.cpp
    src/core/console.cpp
    src/core/logger.cpp
    src/core/window.cpp
    src/core/string.cpp
    src/core/status.cpp
    src/core/skybox.cpp
    src/core/clock.cpp
    src/core/scene.cpp
    src/core/input.cpp
)

set(binary
    src/core/binary/gHeader.cpp
)

set(SCRIPT_SRC
    src/script/meshRender.cpp
    src/script/meshFilter.cpp
    src/script/default.cpp
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
    src/core/task/runtime/textureloadtask.cpp

    src/core/task/texturemanager.cpp
)

set(managers_src
    src/managers/materialmanager.cpp
    src/managers/eventManager.cpp
    src/managers/bvhtreemanager.cpp
    src/managers/octreemanager.cpp
    src/managers/assetsManager.cpp
    src/managers/shadermanager.cpp
    src/managers/shaderprefabmanager.cpp
    src/managers/sceneManager.cpp
    src/managers/meshmanager.cpp
)

set(ECS_SRC
    src/ecs/entityManager.cpp
    src/ecs/archetypeStorage.cpp
    src/ecs/world.cpp
    src/ecs/type.cpp
)

set(EXTERNAL_SRC
    src/externals/hash.cpp
    src/externals/array.c
)

set(ASSET_SRC
    src/asset/materialasset.cpp
    src/asset/shaderasset.cpp
    src/asset/modelasset.cpp
    src/asset/meshasset.cpp
)

set(TASK_SRC
    src/task/shaderbuildtask.cpp
    src/task/shadersavetask.cpp

    src/task/modelloadtask.cpp
    src/task/modelsavetask.cpp

    src/task/materialsavetask.cpp
    src/task/meshsavetask.cpp
)

set(SYSTEM_SRC
    src/system/transformsystem.cpp
    src/system/camerasystem.cpp
    src/system/renderSystem.cpp
    src/system/worldsystem.cpp
)

set(PREFAB_SRC
    src/prefab/shaderprefab.cpp
)

find_package(Bullet QUIET)
if(NOT Bullet_FOUND)
    message(FATAL_ERROR "Please install libbullet-dev")
    return()
endif()

add_subdirectory(imgui)

set(SRC
    src/application.cpp
    src/runtime.cpp
)

add_library(${PROJECT_NAME} SHARED
    ${SCRIPT_SRC}
    ${geometry}
    ${file}
    ${core}
    ${culling}
    ${task}
    ${binary}
    ${animation}
    ${physics}
    ${thread}

    ${managers_src}
    ${EXTERNAL_SRC}
    ${SYSTEM_SRC}
    ${PREFAB_SRC}
    ${ASSET_SRC}
    ${TASK_SRC}
    ${ECS_SRC}
    ${SRC}
)

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    ${BULLET_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE ${BULLET_LIBRARIES} imgui)

target_compile_definitions(${PROJECT_NAME} PUBLIC INTERNAL_REGISTER_SCRIPT)

