cmake_minimum_required(VERSION 3.31.5)

project(gr-base VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

find_package(Bullet QUIET)
if(NOT Bullet_FOUND)
    message(FATAL_ERROR "Please install libbullet-dev")
    return()
endif()

add_subdirectory(imgui)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    ${BULLET_INCLUDE_DIRS}
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
    )

    add_link_options(
        -fsanitize=address,undefined
    )
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE ${BULLET_LIBRARIES} imgui)

target_compile_definitions(${PROJECT_NAME} PUBLIC INTERNAL_REGISTER_SCRIPT)

