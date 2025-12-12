#pragma once

#include <memory>
#include <string>

#include <glm/glm.hpp>

#include <Scene/Scene.hpp>
#include <Scene/Entity.hpp>
#include <Scene/Components.hpp>
#include <Rendering/Framebuffer/Framebuffer.hpp>
#include <Rendering/Renderer.hpp>
#include <Rendering/Buffers/VertexArray.hpp>
#include <Rendering/Shaders/Shader.hpp>
#include <Rendering/Camera/EditorCamera.hpp>
#include "Core/Layer.hpp"

class EditorLayer : public Layer
{
public:
    EditorLayer();
    ~EditorLayer();
    EditorCamera& GetCamera() { return m_EditorCamera; } // camera and input system

    bool m_ShowThemePanel = false;

    void OnAttach() override;
    void OnDetach() override;
    void OnUpdate(float deltaTime) override;
    void ToggleThemePanel() { m_ShowThemePanel = !m_ShowThemePanel; }
    void DrawThemePanel();
    void OnImGuiRender(); // Called every frame to draw panels

private:
    // Scene / ECS
    std::unique_ptr<Scene> m_ActiveScene;
    Entity                 m_SelectedEntity;

    // Rendering
    std::unique_ptr<Framebuffer> m_Framebuffer;
    glm::vec2                    m_ViewportSize = { 1280.0f, 720.0f };

    std::unique_ptr<Shader>      m_Shader;
    
    EditorCamera                 m_EditorCamera;

    // Internal helpers
    void DrawHierarchyPanel();
    void DrawInspectorPanel();
    void DrawContentBrowserPanel();
    void DrawViewportPanel();
};
