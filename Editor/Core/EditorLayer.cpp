#include "EditorLayer.hpp"

#include <imgui.h>
#include <glm/glm.hpp>            //  REQUIRED FIRST
#include <glm/gtc/type_ptr.hpp>   //  THEN this


EditorLayer::EditorLayer()
{
}

EditorLayer::~EditorLayer()
{
}

void EditorLayer::OnAttach()
{
    // Create a new Scene and add some test entities
    m_ActiveScene = std::make_unique<Scene>();

    // Create some sample entities with Tag + Transform
    {
        Entity camera = m_ActiveScene->CreateEntity("Camera");
        camera.AddComponent<TagComponent>("Camera");
        camera.AddComponent<TransformComponent>();

        Entity light = m_ActiveScene->CreateEntity("Directional Light");
        light.AddComponent<TagComponent>("Directional Light");
        light.AddComponent<TransformComponent>();

        Entity cube = m_ActiveScene->CreateEntity("Cube");
        cube.AddComponent<TagComponent>("Cube");
        cube.AddComponent<TransformComponent>();

        Entity floor = m_ActiveScene->CreateEntity("Floor");
        floor.AddComponent<TagComponent>("Floor");
        floor.AddComponent<TransformComponent>();
    }

    // Create framebuffer
    m_Framebuffer = std::make_unique<Framebuffer>(1280, 720);
    Renderer::Init();
}

void EditorLayer::OnDetach()
{
    m_ActiveScene.reset();
}

void EditorLayer::OnImGuiRender()
{
    // Common panel look
    ImGuiWindowFlags panelFlags =
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);

    DrawHierarchyPanel();
    DrawInspectorPanel();
    DrawContentBrowserPanel();
    DrawViewportPanel();

    ImGui::PopStyleVar(2);
}

// ------------------------- PANELS -------------------------

void EditorLayer::DrawHierarchyPanel()
{
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus;

    if (ImGui::Begin("Hierarchy", nullptr, flags))
    {
        ImGui::TextDisabled("Scene");
        ImGui::Separator();
        ImGui::Spacing();

        if (!m_ActiveScene)
        {
            ImGui::Text("No active scene.");
            ImGui::End();
            return;
        }

        // Iterate over all entities that have a TagComponent
        auto& reg = m_ActiveScene->Reg();
        reg.view<TagComponent>().each([&](auto entityHandle, TagComponent& tag)
            {
                Entity entity(entityHandle, m_ActiveScene.get());

                // Highlight if this entity is currently selected
                bool isSelected = (m_SelectedEntity.Handle() == entityHandle);

                ImGuiTreeNodeFlags nodeFlags =
                    ImGuiTreeNodeFlags_Leaf |
                    ImGuiTreeNodeFlags_NoTreePushOnOpen |
                    ImGuiTreeNodeFlags_SpanAvailWidth;
                if (isSelected)
                    nodeFlags |= ImGuiTreeNodeFlags_Selected;

                ImGui::TreeNodeEx((void*)(uint64_t)entityHandle, nodeFlags, "%s", tag.Tag.c_str());

                if (ImGui::IsItemClicked())
                {
                    m_SelectedEntity = entity;
                }
            });
    }
    ImGui::End();
}

void EditorLayer::DrawInspectorPanel()
{
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus;

    if (ImGui::Begin("Inspector", nullptr, flags))
    {
        ImGui::TextDisabled("Inspector");
        ImGui::Separator();
        ImGui::Spacing();

        if (!m_SelectedEntity || !m_ActiveScene)
        {
            ImGui::Text("No entity selected.");
            ImGui::End();
            return;
        }

        auto& reg = m_ActiveScene->Reg();
        entt::entity handle = m_SelectedEntity.Handle();

        // Tag (name)
        if (reg.any_of<TagComponent>(handle))
        {
            auto& tag = reg.get<TagComponent>(handle);

            char buffer[256];
            memset(buffer, 0, sizeof(buffer));
            strncpy(buffer, tag.Tag.c_str(), sizeof(buffer) - 1);

            ImGui::Text("Name");
            ImGui::SameLine();
            if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
            {
                tag.Tag = std::string(buffer);
            }

            ImGui::Separator();
        }

        // Transform
        if (reg.any_of<TransformComponent>(handle))
        {
            auto& transform = reg.get<TransformComponent>(handle);

            ImGui::Text("Transform");
            ImGui::Separator();

            ImGui::Text("Position");
            ImGui::SameLine();
            ImGui::DragFloat3("##Position", glm::value_ptr(transform.Position), 0.1f);

            ImGui::Text("Rotation");
            ImGui::SameLine();
            ImGui::DragFloat3("##Rotation", glm::value_ptr(transform.Rotation), 0.1f);

            ImGui::Text("Scale");
            ImGui::SameLine();
            ImGui::DragFloat3("##Scale", glm::value_ptr(transform.Scale), 0.1f);
        }
    }
    ImGui::End();
}

void EditorLayer::DrawContentBrowserPanel()
{
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus;

    if (ImGui::Begin("Content Browser", nullptr, flags))
    {
        ImGui::TextDisabled("Assets");
        ImGui::Separator();
        ImGui::Spacing();

        // TEMP: fake assets
        if (ImGui::Selectable("Meshes/")) {}
        if (ImGui::Selectable("Textures/")) {}
        if (ImGui::Selectable("Scenes/")) {}
    }
    ImGui::End();
}

void EditorLayer::DrawViewportPanel()
{
    ImGuiWindowFlags viewportFlags =
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse;

    if (ImGui::Begin("Viewport", nullptr, viewportFlags))
    {
        // Determine available size
        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

        // Resize framebuffer if needed
        if ((uint32_t)m_ViewportSize.x > 0 &&
            (uint32_t)m_ViewportSize.y > 0 &&
            (m_Framebuffer->GetWidth() != (uint32_t)m_ViewportSize.x ||
                m_Framebuffer->GetHeight() != (uint32_t)m_ViewportSize.y))
        {
            m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
        }

        // Render into framebuffer
        m_Framebuffer->Bind();
        Renderer::Clear({ 0.12f, 0.12f, 0.14f, 1.0f });

        // (Later: render 3D scene here)

        m_Framebuffer->Unbind();

        // Draw framebuffer texture in ImGui
        uint32_t textureID = m_Framebuffer->GetColorAttachment();
        ImGui::Image((void*)(intptr_t)textureID, viewportPanelSize, ImVec2(0, 1), ImVec2(1, 0));
    }
    ImGui::End();
}

