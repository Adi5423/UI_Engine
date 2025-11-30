#include <Core/Application.hpp>
#include <Core/Window.hpp>

#include "Core/ImGuiLayer.hpp"
#include "Core/EditorLayer.hpp"

// MOST IMPORTANT — Editor must include imgui.h
#include <imgui.h>

int main()
{
    Application app;

    // Acquire window from engine (now public)
    GLFWwindow* window = (GLFWwindow*)app.GetWindow()->GetNativeWindow();

    // Layers
    ImGuiLayer imgui;
    EditorLayer editor;

    imgui.OnAttach(window);
    editor.OnAttach();

    bool running = true;

    while (running)
    {
        glClearColor(0.1f, 0.1f, 0.1f, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        imgui.Begin();

        // --- Dockspace ---
        {
            static bool dockspaceOpen = true;
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

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            ImGui::Begin("##DockspaceRoot", &dockspaceOpen, windowFlags);
            ImGui::PopStyleVar();

            ImGuiID dockspaceID = ImGui::GetID("MainDockspace");
            ImGui::DockSpace(dockspaceID, ImVec2(0, 0), dockFlags);

            if (ImGui::BeginMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    if (ImGui::MenuItem("Exit"))
                        running = false;

                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }

            ImGui::End();
        }
        // -----------------

        editor.OnImGuiRender();

        imgui.End();

        app.GetWindow()->OnUpdate();
    }

    return 0;
}
