#pragma once

#include <memory>
#include <string>

#include <Scene/Scene.hpp>
#include <Scene/Entity.hpp>
#include <Scene/Components.hpp>
#include <Rendering/Framebuffer.hpp>
#include <Rendering/Renderer.hpp>


class EditorLayer
{
public:
    EditorLayer();
    ~EditorLayer();

    void OnAttach();
    void OnDetach();
    void OnImGuiRender(); // Called every frame to draw panels

private:
    // Panels will operate on this active scene
    std::unique_ptr<Scene> m_ActiveScene;

    std::unique_ptr<Framebuffer> m_Framebuffer;
    glm::vec2 m_ViewportSize = { 1280.0f, 720.0f };

    // Currently selected entity in Hierarchy
    Entity m_SelectedEntity;

    // Internal helpers
    void DrawHierarchyPanel();
    void DrawInspectorPanel();
    void DrawContentBrowserPanel();
    void DrawViewportPanel();
};
