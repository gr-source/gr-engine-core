cmake_minimum_required(VERSION 3.31.5)

project(gr-base VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

find_package(Bullet QUIET)
if(NOT Bullet_FOUND)
    message(FATAL_ERROR "Please install libbullet-dev")
    return()
endif()

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

set(managers_src
    src/managers/materialmanager.cpp
    src/managers/eventManager.cpp
    src/managers/assetsManager.cpp
    src/managers/shadermanager.cpp
    src/managers/shaderprefabmanager.cpp

    src/managers/textureManager.cpp
    src/managers/sceneManager.cpp
    src/managers/meshmanager.cpp
)

set(ECS_SRC
    src/ecs/componentRegistry.cpp
    src/ecs/archetypeStorage.cpp
    src/ecs/systemRegistry.cpp
    src/ecs/entityManager.cpp
    src/ecs/world.cpp
)

set(EXTERNAL_SRC
    src/externals/hash.cpp
    src/externals/array.c
)

set(ASSET_SRC
    src/asset/assetResolver.cpp

    src/asset/materialasset.cpp
    src/asset/textureAsset.cpp
    src/asset/shaderasset.cpp
    src/asset/modelasset.cpp
    src/asset/meshasset.cpp
    src/asset/assetPtr.cpp
    src/asset/URI.cpp
)

set(TASK_SRC
    src/task/shaderbuildtask.cpp
    src/task/shadersavetask.cpp

    src/task/modelloadtask.cpp
    src/task/modelsavetask.cpp

    src/task/materialsavetask.cpp
    src/task/meshsavetask.cpp

    src/task/textureBuildTask.cpp
    src/task/textureSaveTask.cpp
)

set(SYSTEM_SRC
    src/system/transformsystem.cpp
    src/system/camerasystem.cpp
    src/system/renderSystem.cpp
    src/system/default.cpp
)

set(PREFAB_SRC
    src/prefab/shaderprefab.cpp
)

set(SRC
    src/application.cpp
    src/runtime.cpp
)

set(SPATIAL_SRC
    src/spatial/bvhtree.cpp
)

set(DEBUG_SRC
    src/debug/debugBuffer.cpp
    src/debug/debugDraw.cpp
)

set(MODULE_SRC
    src/module/moduleloader.cpp
)

add_subdirectory(imgui)

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
    ${SPATIAL_SRC}
    ${MODULE_SRC}
    ${SYSTEM_SRC}
    ${PREFAB_SRC}
    ${DEBUG_SRC}
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

