cmake_minimum_required(VERSION 3.10)
project(MiniMotor)

# Evita el warning de CMake sobre la preferencia GLVND/legacy para OpenGL.
if(POLICY CMP0072)
  cmake_policy(SET CMP0072 NEW)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)

# ------------------------------------------------------------
# Organización del proyecto:
#   include/  -- todos los .h propios del motor
#   src/      -- todos los .cpp propios del motor
#   imgui/    -- librería de terceros, no se toca
# Los .cpp de src/ siguen incluyendo sus headers con comillas simples
# (por ejemplo, #include "GameObject.h"): no hace falta escribir
# "../include/GameObject.h" en ningún lado, porque target_include_directories
# más abajo agrega include/ al path de búsqueda del compilador.
# ------------------------------------------------------------
add_executable(MiniMotor
    src/main.cpp
    src/GameObject.cpp
    src/Engine.cpp
    src/Collision.cpp
    src/Renderer.cpp
    src/Player.cpp
    src/Enemy.cpp
    src/Bullet.cpp
    src/DebugDraw.cpp
    src/ImGuiLayer.cpp
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_opengl3.cpp
)

target_include_directories(MiniMotor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends
)

target_link_libraries(MiniMotor PRIVATE glfw GLEW::GLEW OpenGL::GL)
