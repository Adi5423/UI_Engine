set(ENGINE_SRC
    Core/Application.cpp
    Core/GLFWWindow.cpp

    Rendering/Renderer.cpp
    Rendering/Framebuffer.cpp

    Scene/Scene.cpp
)

add_library(UICheckEngine STATIC ${ENGINE_SRC} 
        "Rendering/Shader.cpp" 
        "Rendering/Buffer.cpp" 
        "Rendering/VertexArray.cpp" 
        "Rendering/EditorCamera.cpp"

)

target_include_directories(UICheckEngine PUBLIC
    ${CMAKE_SOURCE_DIR}/Engine
    ${CMAKE_SOURCE_DIR}/vendor
)

target_link_libraries(UICheckEngine PUBLIC
    glad
    glfw
    opengl32
)
