#include "EditorApplication.hpp"

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <glm/glm.hpp>

#include "ImGuiLayer.hpp"
#include "EditorLayer.hpp"
#include "ThemeSettings.hpp"

#include <Core/Input/Input.hpp>
#include <Core/Input/ViewportInput.hpp>

EditorApplication::EditorApplication(const ApplicationSpecification& spec)
    : Application(spec)
{
    // Layers will be created in OnInit
}

EditorApplication::~EditorApplication()
{
    // Cleanup handled by OnShutdown and smart pointers
}

void EditorApplication::OnInit()
{
    // Get GLFW window from engine
    GLFWwindow* window = (GLFWwindow*)GetWindow()->GetNativeWindow();

    // ========================================================================
    // Initialize Input Systems
    // ========================================================================
    Input::Init(window);
    ViewportInput::Init(window);

    // ========================================================================
    // Create and Attach Layers
    // ========================================================================
    m_ImGuiLayer = new ImGuiLayer();
    PushOverlay(m_ImGuiLayer);

    m_EditorLayer = new EditorLayer();
    PushLayer(m_EditorLayer);

    std::cout << "[Editor] Layers initialized.\n";
}

void EditorApplication::OnUpdate(float deltaTime)
{
    // ========================================================================
    // Clear Screen
    // ========================================================================
    glClearColor(0.1f, 0.1f, 0.1f, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    // NOTE: Camera/Input Logic moved to EditorLayer::OnUpdate(), called by Application Stack.

    // ========================================================================
    // ImGui Rendering
    // ========================================================================
    m_ImGuiLayer->Begin();

    RenderDockspace();

    // Render all layers ImGui
    for (Layer* layer : GetLayerStack())
        layer->OnImGuiRender();

    m_ImGuiLayer->End();
}

void EditorApplication::OnShutdown()
{
    std::cout << "[Editor] Shutting down layers...\n";
    // Layers are deleted by LayerStack in Application destructor
    m_EditorLayer = nullptr;
    m_ImGuiLayer = nullptr;
}

void EditorApplication::RenderDockspace()
{
    static ImGuiDockNodeFlags dockFlags = ImGuiDockNodeFlags_None;

    ImGuiWindowFlags windowFlags =
        ImGuiWindowFlags_MenuBar |
        ImGuiWindowFlags_NoDocking;

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);

    windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::Begin("##DockspaceRoot", &m_ShowDockspace, windowFlags);

    ImGuiID dockspaceID = ImGui::GetID("MainDockspace");
    ImGui::DockSpace(dockspaceID, ImVec2(0, 0), dockFlags);

    RenderMenuBar();

    ImGui::End();
}

void EditorApplication::RenderMenuBar()
{
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Exit"))
                Close();

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Settings"))
        {
            if (ImGui::MenuItem("Theme"))
                m_EditorLayer->ToggleThemePanel();

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }
}
