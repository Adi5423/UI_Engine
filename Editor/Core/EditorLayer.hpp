#pragma once

#include <memory>
#include <string>

#include <glm/glm.hpp>

#include <Scene/Scene.hpp>
#include <Scene/Entity.hpp>
#include <Scene/Components.hpp>
#include <Rendering/Framebuffer.hpp>
#include <Rendering/Renderer.hpp>
#include <Rendering/VertexArray.hpp>
#include <Rendering/Shader.hpp>
#include <Rendering/EditorCamera.hpp>

class EditorLayer
{
public:
    EditorLayer();
    ~EditorLayer();

    void OnAttach();
    void OnDetach();
    void OnImGuiRender(); // Called every frame to draw panels

private:
    // Scene / ECS
    std::unique_ptr<Scene> m_ActiveScene;
    Entity                 m_SelectedEntity;

    // Rendering
    std::unique_ptr<Framebuffer> m_Framebuffer;
    glm::vec2                    m_ViewportSize = { 1280.0f, 720.0f };

    std::unique_ptr<VertexArray> m_CubeVA;
    std::unique_ptr<Shader>      m_Shader;
    EditorCamera                 m_EditorCamera;

    // Internal helpers
    void DrawHierarchyPanel();
    void DrawInspectorPanel();
    void DrawContentBrowserPanel();
    void DrawViewportPanel();
};
