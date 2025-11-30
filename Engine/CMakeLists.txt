set(ENGINE_SRC
    Core/Application.cpp
    Core/GLFWWindow.cpp

    Rendering/Renderer.cpp
    Rendering/Framebuffer.cpp

    Scene/Scene.cpp
)

add_library(UICheckEngine STATIC ${ENGINE_SRC})

target_include_directories(UICheckEngine PUBLIC
    ${CMAKE_SOURCE_DIR}/Engine
    ${CMAKE_SOURCE_DIR}/vendor
)

target_link_libraries(UICheckEngine PUBLIC
    glad
    glfw
    opengl32
)
