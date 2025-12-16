#pragma once

#include <memory>
#include <Scene/Scene.hpp>
#include <Scene/Entity.hpp>
#include <Rendering/Camera/EditorCamera.hpp>
#include <Rendering/Framebuffer/Framebuffer.hpp>
#include <Rendering/Shaders/Shader.hpp>

class SceneRenderer
{
public:
    SceneRenderer();
    ~SceneRenderer() = default;

    void Init();
    
    // Resize the internal framebuffer
    void SetViewportSize(uint32_t width, uint32_t height);

    // Main render function for the editor
    void RenderEditor(Scene* scene, const EditorCamera& camera, Entity selectedEntity = {});

    // Get the final output texture ID (for ImGui)
    uint32_t GetFinalImage();

    // Accessor to the framebuffer in case we need it
    std::shared_ptr<Framebuffer> GetFramebuffer() const { return m_Framebuffer; }

private:
    std::shared_ptr<Framebuffer> m_Framebuffer;
    std::shared_ptr<Shader> m_Shader; // Basic shader for now

    uint32_t m_ViewportWidth = 1280;
    uint32_t m_ViewportHeight = 720;
};
