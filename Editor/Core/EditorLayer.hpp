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
#include <Rendering/SceneRenderer.hpp>
#include <Rendering/Camera/EditorCamera.hpp>
#include <Core/Commands/CommandHistory.hpp>
#include <Core/Commands/SceneCommands.hpp>
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
    Entity m_SelectedEntity;

    // Rendering
    std::shared_ptr<SceneRenderer> m_SceneRenderer;
    glm::vec2 m_ViewportSize = { 1280.0f, 720.0f };
    
    EditorCamera m_EditorCamera;
    
    int m_GizmoType = 0; // ImGuizmo::OPERATION

    // Command History for Undo/Redo
    CommandHistory m_CommandHistory;

    // State tracking for undo/redo
    struct {
        TransformComponent savedTransform;
        bool isEditingPosition = false;
        bool isEditingRotation = false;
        bool isEditingScale = false;
    } m_TransformEditState;
    
    std::string m_PreviousName;
    bool m_IsEditingName = false;
    bool m_WasUsingGizmo = false;
    int m_GizmoTypeAtStart = -1;

    // Gizmo preview (avoid matrix->euler->matrix feedback during manipulation)
    bool m_GizmoPreviewActive = false;
    entt::entity m_GizmoPreviewEntity = entt::null;
    glm::mat4 m_GizmoPreviewMatrix{ 1.0f };

    // Shortcuts state
    bool m_UndoPressedLastFrame = false;
    bool m_RedoPressedLastFrame = false;

    // Clipboard State
    enum class ClipboardMode { None, Copy, Cut };
    struct ClipboardRecord {
        ClipboardMode Mode = ClipboardMode::None;
        Core::UUID EntityID = 0;
    } m_Clipboard;

    // Delete Popup State
    bool m_ShowDeletePopup = false;
    glm::vec2 m_DeletePopupPos = { 0.0f, 0.0f };
    glm::vec3 m_DeletePopupWorldPos = { 0.0f, 0.0f, 0.0f };
    bool m_DeletePopupNeedsPositioning = false;
    entt::entity m_CutEntityID = entt::null; // For visual fading in hierarchy

    // Internal helpers
    void DrawHierarchyPanel();
    void DrawInspectorPanel();
    void DrawContentBrowserPanel();
    void DrawViewportPanel();
    
    // Utility
    glm::vec2 WorldToScreen(const glm::vec3& worldPos, const glm::mat4& view, 
                            const glm::mat4& proj, const glm::vec2& viewportSize, 
                            const glm::vec2& viewportPos);
};
