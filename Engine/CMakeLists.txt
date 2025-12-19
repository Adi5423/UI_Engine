set(ENGINE_SRC
    Core/Application.cpp
    Core/GLFWWindow.cpp
    Core/Input/Input.cpp
    Core/Input/ViewportInput.cpp
    Core/Log.cpp
    Core/Resources/ResourceManager.cpp
    Core/Layer.cpp
    Core/LayerStack.cpp
    Core/UUID.cpp

    Rendering/Mesh/Mesh.cpp

    Rendering/Renderer.cpp
    Rendering/SceneRenderer.cpp
    Rendering/Framebuffer/Framebuffer.cpp

    Scene/Scene.cpp
)

add_library(UICheckEngine SHARED ${ENGINE_SRC} 
        "Rendering/Shaders/Shader.cpp" 
        "Rendering/Buffers/Buffer.cpp" 
        "Rendering/Buffers/VertexArray.cpp" 
        "Rendering/Camera/EditorCamera.cpp"  
)
set_target_properties(UICheckEngine PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE PREFIX "")

target_include_directories(UICheckEngine PUBLIC
    ${CMAKE_SOURCE_DIR}/Engine
    ${CMAKE_SOURCE_DIR}/vendor
)

find_package(OpenGL REQUIRED)

target_link_libraries(UICheckEngine PUBLIC
    glad
    glfw
    OpenGL::GL
)
