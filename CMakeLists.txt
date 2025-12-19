cmake_minimum_required(VERSION 3.20)
project(UICheck LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ----------- vendor libs -----------
add_library(glad SHARED vendor/glad/src/glad.c)
set_target_properties(glad PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE PREFIX "")
target_include_directories(glad PUBLIC vendor/glad/include)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
add_subdirectory(vendor/glfw)

set(IMGUI_DIR vendor/imgui)
add_library(imgui SHARED
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)
set_target_properties(imgui PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE PREFIX "")
target_include_directories(imgui PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)
target_link_libraries(imgui PUBLIC glad glfw)

# ImGuizmo
set(IMGUIZMO_DIR vendor/ImGuizmo)
add_library(ImGuizmo SHARED
    ${IMGUIZMO_DIR}/ImGuizmo.cpp
)
set_target_properties(ImGuizmo PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE PREFIX "")
target_include_directories(ImGuizmo PUBLIC ${IMGUIZMO_DIR})
target_link_libraries(ImGuizmo PUBLIC imgui)

include_directories(
    vendor/glm
    vendor/stb
    vendor/entt/single_include
    vendor/json
)

# ---------- engine & editor ------------
add_subdirectory(Engine)
add_subdirectory(Editor)

# ---------- install / release packaging ------------
install(TARGETS UICheckEditor UICheckEngine glfw glad imgui ImGuizmo
    RUNTIME DESTINATION .
    LIBRARY DESTINATION .
    ARCHIVE DESTINATION lib
)

# Custom install for system DLLs (MinGW)
if(WIN32 AND NOT MSVC)
    get_filename_component(COMPILER_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
    set(MINGW_LIBS
        libgcc_s_seh-1.dll
        libstdc++-6.dll
        libwinpthread-1.dll
    )
    foreach(lib ${MINGW_LIBS})
        if(EXISTS "${COMPILER_DIR}/${lib}")
            install(FILES "${COMPILER_DIR}/${lib}" DESTINATION .)
        endif()
    endforeach()
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/Drop_at_EXE")
    install(DIRECTORY "${CMAKE_SOURCE_DIR}/Drop_at_EXE/" DESTINATION .)
endif()
