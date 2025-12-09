#include <glad/glad.h>
#include "Core/Application.hpp"
#include "Core/Window.hpp"

#include "Core/ImGuiLayer.hpp"
#include "Core/EditorLayer.hpp"

#include "Core/Input/Input.hpp"
#include "Core/Input/ViewportInput.hpp"

// Including inside Editor imgui.h
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>


int main()
{
    Application app;

    // Acquire window from engine (now public)
    GLFWwindow* window = (GLFWwindow*)app.GetWindow()->GetNativeWindow();
    // Initialize Input system
    Input::Init(window);
    ViewportInput::Init(window);

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

        // ---- Viewport-Scoped Camera Input ----

        // Update camera activation state based ONLY on RMB press
        ViewportInput::UpdateCameraState(Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT));

        if (ViewportInput::IsCameraActive())
        {
            static float lastTime = (float)glfwGetTime();
            float time = (float)glfwGetTime();
            float dt = time - lastTime;
            lastTime = time;
        
            glm::vec3 dir{0.0f};
        
            if (Input::IsKeyPressed(GLFW_KEY_W)) dir.z += 1.0f;
            if (Input::IsKeyPressed(GLFW_KEY_S)) dir.z -= 1.0f;
            if (Input::IsKeyPressed(GLFW_KEY_A)) dir.x -= 1.0f;
            if (Input::IsKeyPressed(GLFW_KEY_D)) dir.x += 1.0f;
            if (Input::IsKeyPressed(GLFW_KEY_E)) dir.y += 1.0f;
            if (Input::IsKeyPressed(GLFW_KEY_Q)) dir.y -= 1.0f;
        
            double dx, dy;
            ViewportInput::GetMouseDelta(dx, dy);
        
            editor.GetCamera().ProcessKeyboard(dir, dt);
            editor.GetCamera().ProcessMouseMovement((float)dx, (float)dy);
        }


        //engine initialization via imgui
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
