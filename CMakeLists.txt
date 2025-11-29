cmake_minimum_required(VERSION 3.20)
project(UICheck VERSION 0.1 LANGUAGES C CXX)

# -------------------------------------------------------
# Global settings
# -------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# -------------------------------------------------------
# Include vendor libraries
# -------------------------------------------------------

# GLAD --------------------------------------------------
add_library(glad STATIC
    vendor/glad/src/glad.c
)
target_include_directories(glad PUBLIC vendor/glad/include)

# GLFW --------------------------------------------------
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(vendor/glfw)

# IMGUI --------------------------------------------------
set(IMGUI_DIR vendor/imgui)

add_library(imgui STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_tables.cpp

    # Backend for GLFW + OpenGL3
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${PROJECT_SOURCE_DIR}/vendor/glfw/include
)

# GLM, STB, ENTT (header-only libs) ---------------------
include_directories(
    vendor/glm
    vendor/stb
    vendor/entt/single_include
)

# -------------------------------------------------------
# Source files (we'll add more later)
# -------------------------------------------------------
file(GLOB_RECURSE SRC_FILES src/*.cpp)

add_executable(UICheck ${SRC_FILES})

target_include_directories(UICheck PUBLIC src)

target_link_libraries(UICheck
    glad
    glfw
    imgui
    opengl32
)
