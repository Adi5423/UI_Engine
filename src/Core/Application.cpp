#include "Application.hpp"
#include "Window.hpp"
#include "ImGuiLayer.hpp"
#include <imgui.h>
#include <glad/glad.h>
#include <iostream>


Application::Application()
{
    m_Window = Window::Create();
    m_ImGuiLayer = new ImGuiLayer();
    m_ImGuiLayer->OnAttach((GLFWwindow*)m_Window->GetNativeWindow());
    
    m_EditorLayer = new EditorLayer();
    m_EditorLayer->OnAttach();

}

Application::~Application()
{
    m_EditorLayer->OnDetach();
    delete m_EditorLayer;

    delete m_Window;
}


void Application::Run()
{
    while (m_Running)
    {
        // Clear screen
        glClearColor(0.1f, 0.1f, 0.1f, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        // Begin ImGui frame
        m_ImGuiLayer->Begin();

        // ------------------ ROOT DOCKSPACE ------------------------
        {
            static bool dockspaceOpen = true;
            static bool opt_fullscreen = true;
            static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

            ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
            if (opt_fullscreen)
            {
                ImGuiViewport* viewport = ImGui::GetMainViewport();
                ImGui::SetNextWindowPos(viewport->Pos);
                ImGui::SetNextWindowSize(viewport->Size);
                ImGui::SetNextWindowViewport(viewport->ID);
                window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
                window_flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
                window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
            }

            // Flat, full-screen root window with no padding
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

            ImGui::Begin("##RootDockSpace", &dockspaceOpen, window_flags);
            ImGui::PopStyleVar(3);

            // Dockspace node
            ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0, 0), dockspace_flags);

            // Top menu bar
            if (ImGui::BeginMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    if (ImGui::MenuItem("Exit"))
                        m_Running = false;

                    ImGui::EndMenu();
                }

                // You can add Edit/View/Help here later

                ImGui::EndMenuBar();
            }

            ImGui::End();
        }
        // ----------------------------------------------------------

        // Draw all editor panels *inside* that dockspace
        m_EditorLayer->OnImGuiRender();

        // End ImGui frame (render)
        m_ImGuiLayer->End();

        m_Window->OnUpdate();
    }
}

