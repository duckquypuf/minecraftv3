cmake_minimum_required(VERSION 4.1.2)
project(minecraft)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -DNDEBUG -flto")

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

include_directories(include src thirdparty thirdparty/imgui thirdparty/imgui/backends)

set(IMGUI_SOURCES
    thirdparty/imgui/imgui.cpp
    thirdparty/imgui/imgui_draw.cpp
    thirdparty/imgui/imgui_tables.cpp
    thirdparty/imgui/imgui_widgets.cpp
    thirdparty/imgui/backends/imgui_impl_glfw.cpp
    thirdparty/imgui/backends/imgui_impl_opengl3.cpp
)

add_executable(minecraft
    src/main.cpp
    src/chunk.cpp
    src/camera.cpp
    src/stb_impl.cpp
    thirdparty/glad.c
    ${IMGUI_SOURCES}
)

target_link_libraries(minecraft
    glfw
    OpenGL::GL
)

add_definitions(-Wno-deprecated-declarations)
