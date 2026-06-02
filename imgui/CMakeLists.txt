project("imgui")

set(imgui_src
    backends/imgui_impl_opengl3.cpp
    backends/imgui_impl_glfw.cpp

    texteditor/textEditor.cpp
    
    imgui_widgets.cpp
    imgui_tables.cpp
    imgui_demo.cpp
    imgui_draw.cpp
    imgui.cpp
)

add_library(${PROJECT_NAME} SHARED ${imgui_src})

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/${PROJECT_NAME}>
)


