cmake_minimum_required(VERSION 3.31.5)

project(gr-base VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Bullet QUIET)
if(NOT Bullet_FOUND)
  message(FATAL_ERROR "Please install libbullet-dev")
  return()
endif()

find_package(GLEW QUIET)
if(NOT GLEW_FOUND)
  message(FATAL_ERROR "Please install GLEW.")
  return()
endif()

find_package(glfw3 QUIET)
if(NOT glfw3_FOUND)
    message(FATAL_ERROR "Please install libglfw3-dev")
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

target_link_libraries(${PROJECT_NAME} PRIVATE ${BULLET_LIBRARIES} ${GLEW_LIBRARIES} imgui)

target_compile_definitions(${PROJECT_NAME} PUBLIC INTERNAL_REGISTER_SCRIPT)

