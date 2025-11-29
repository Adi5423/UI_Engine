#include "EditorLayer.hpp"
#include <imgui.h>

EditorLayer::EditorLayer() {}
EditorLayer::~EditorLayer() {}

void EditorLayer::OnAttach() {}
void EditorLayer::OnDetach() {}

void EditorLayer::OnImGuiRender()
{
    // Common flags for engine-like panels
    ImGuiWindowFlags panelFlags =
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus;

    // Optional: slight style tweak only for panels
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);

    // --- Scene Hierarchy (left)
    if (ImGui::Begin("Hierarchy", nullptr, panelFlags))
    {
        ImGui::TextDisabled("Scene");
        ImGui::Separator();
        ImGui::Spacing();

        // TEMP: placeholder items
        ImGui::Selectable("Camera");
        ImGui::Selectable("Directional Light");
        ImGui::Selectable("Cube");
        ImGui::Selectable("Floor");
    }
    ImGui::End();

    // --- Inspector (right)
    if (ImGui::Begin("Inspector", nullptr, panelFlags))
    {
        ImGui::TextDisabled("Inspector");
        ImGui::Separator();
        ImGui::Spacing();

        // TEMP: placeholder properties
        ImGui::Text("Selected: Cube");
        ImGui::Separator();

        ImGui::Text("Transform");
        ImGui::DragFloat3("Position", (float[3]) { 0.0f, 1.0f, 0.0f }, 0.1f);
        ImGui::DragFloat3("Rotation", (float[3]) { 0.0f, 0.0f, 0.0f }, 0.1f);
        ImGui::DragFloat3("Scale", (float[3]) { 1.0f, 1.0f, 1.0f }, 0.1f);

        ImGui::Separator();
        ImGui::Text("Material");
        static float color[3] = { 0.8f, 0.3f, 0.2f };
        ImGui::ColorEdit3("Albedo", color);
    }
    ImGui::End();

    // --- Content Browser (bottom)
    if (ImGui::Begin("Content Browser", nullptr, panelFlags))
    {
        ImGui::TextDisabled("Assets");
        ImGui::Separator();
        ImGui::Spacing();

        // TEMP: fake assets
        if (ImGui::Selectable("Meshes/"))
        {
        }
        if (ImGui::Selectable("Textures/"))
        {
        }
        if (ImGui::Selectable("Scenes/"))
        {
        }
    }
    ImGui::End();

    // --- Viewport (center)
    // Later this will show the rendered framebuffer texture
    ImGuiWindowFlags viewportFlags = panelFlags | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
    if (ImGui::Begin("Viewport", nullptr, viewportFlags))
    {
        ImGui::TextDisabled("Viewport");
        ImGui::Separator();

        // For now, show a placeholder canvas area
        ImVec2 size = ImGui::GetContentRegionAvail();
        ImGui::Dummy(size); // just reserve space; later we'll draw ImGui::Image(texture)
    }
    ImGui::End();

    ImGui::PopStyleVar(2);
}
