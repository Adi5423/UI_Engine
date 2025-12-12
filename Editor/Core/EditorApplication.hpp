#pragma once

/**
 * ============================================================================
 * EDITOR APPLICATION
 * ============================================================================
 * 
 * This is the Editor's custom Application class that inherits from the 
 * engine's base Application. This follows the professional pattern where:
 * 
 * - The Engine provides the base Application class
 * - The Editor (client) creates its own EditorApplication that inherits from it
 * - All Editor-specific initialization, update, and shutdown logic lives here
 * 
 * This keeps the Editor's main.cpp clean and simple - just using EntryPoint.hpp
 * and implementing CreateApplication().
 * 
 * ============================================================================
 */

#include <Core/Application.hpp>
#include <memory>

// Forward declarations
class ImGuiLayer;
class EditorLayer;

class EditorApplication : public Application
{
public:
    EditorApplication(const ApplicationSpecification& spec);
    virtual ~EditorApplication();

    // ========================================================================
    // Lifecycle Overrides
    // ========================================================================
    
    virtual void OnInit() override;
    virtual void OnUpdate(float deltaTime) override;
    virtual void OnShutdown() override;

    // ========================================================================
    // Editor-Specific API
    // ========================================================================
    
    EditorLayer* GetEditorLayer() const { return m_EditorLayer.get(); }

private:
    // Layers
    std::unique_ptr<ImGuiLayer> m_ImGuiLayer;
    std::unique_ptr<EditorLayer> m_EditorLayer;

    // State
    bool m_ShowDockspace = true;

    // Internal methods
    void RenderDockspace();
    void RenderMenuBar();
};
