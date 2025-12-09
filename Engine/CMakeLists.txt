set(ENGINE_SRC
    Core/Application.cpp
    Core/GLFWWindow.cpp
    Core/Input/Input.cpp
    Core/Input/ViewportInput.cpp

    Rendering/Mesh/Mesh.cpp

    Rendering/Renderer.cpp
    Rendering/Framebuffer/Framebuffer.cpp

    Scene/Scene.cpp
)

add_library(UICheckEngine STATIC ${ENGINE_SRC} 
        "Rendering/Shaders/Shader.cpp" 
        "Rendering/Buffers/Buffer.cpp" 
        "Rendering/Buffers/VertexArray.cpp" 
        "Rendering/Camera/EditorCamera.cpp"
        
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
