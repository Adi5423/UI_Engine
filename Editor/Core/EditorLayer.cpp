#include "EditorLayer.hpp"
#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>
#include <imgui_internal.h>
#include <ImGuizmo.h>
#include <glad/glad.h>      // for glDrawElements etc.
#include <glm/glm.hpp>

#include <Core/Input/ViewportInput.hpp>
#include <Scene/Scene.hpp>
#include <Scene/Entity.hpp>
#include <Scene/Components.hpp>
#include <Scene/SceneAPI.hpp>
#include <Core/ThemeSettings.hpp>
#include <Core/ImGuiLayer.hpp>

// Helper for Selection
static bool RayIntersectsAABB(glm::vec3 origin, glm::vec3 dir, glm::vec3 minB, glm::vec3 maxB, float& t)
{
    float tmin = (minB.x - origin.x) / dir.x;
    float tmax = (maxB.x - origin.x) / dir.x;

    if (tmin > tmax) std::swap(tmin, tmax);

    float tymin = (minB.y - origin.y) / dir.y;
    float tymax = (maxB.y - origin.y) / dir.y;

    if (tymin > tymax) std::swap(tymin, tymax);

    if ((tmin > tymax) || (tymin > tmax))
        return false;

    if (tymin > tmin) tmin = tymin;
    if (tymax < tmax) tmax = tymax;

    float tzmin = (minB.z - origin.z) / dir.z;
    float tzmax = (maxB.z - origin.z) / dir.z;

    if (tzmin > tzmax) std::swap(tzmin, tzmax);

    if ((tmin > tzmax) || (tzmin > tmax))
        return false;

    if (tzmin > tmin) tmin = tzmin;
    if (tzmax < tmax) tmax = tzmax;

    if (tmax < 0) return false;

    t = tmin;
    if (t < 0) t = tmax;
    return true;
}


EditorLayer::EditorLayer() : Layer("EditorLayer") {}
EditorLayer::~EditorLayer() = default;

void EditorLayer::OnAttach()
{
    // Scene setup
    m_ActiveScene = std::make_unique<Scene>();

    // inspector pannels views.
    {
        // Scene setup via API
        m_ActiveScene = std::make_unique<Scene>();
        SceneAPI::CreateDefaultScene(*m_ActiveScene);

    }

    // Framebuffer
    m_Framebuffer = std::make_unique<Framebuffer>(1280, 720);

    // // ---------- Simple cube geometry ----------
    // float vertices[] = {
    //     // positions (a simple quad in front for now)
    //     -0.5f, -0.5f, -0.5f,
    //      0.5f, -0.5f, -0.5f,
    //      0.5f,  0.5f, -0.5f,
    //     -0.5f,  0.5f, -0.5f,
    // };

    // uint32_t indices[] = {
    //     0, 1, 2,
    //     2, 3, 0
    // };

    // m_CubeVA = std::make_unique<VertexArray>();
    // auto vb = new VertexBuffer(vertices, sizeof(vertices));
    // auto ib = new IndexBuffer(indices, 6);
    // m_CubeVA->AddVertexBuffer(vb);
    // m_CubeVA->SetIndexBuffer(ib);

    // ---------- Simple shader ----------
    std::string vs = R"(
#version 450 core

layout(location = 0) in vec3 aPos;

uniform mat4 u_Model;
uniform mat4 u_ViewProj;

void main()
{
    gl_Position = u_ViewProj * u_Model * vec4(aPos, 1.0);
}

)";

    std::string fs = R"(
#version 450 core

out vec4 FragColor;
uniform vec4 u_Color;

void main()
{
    FragColor = u_Color;
}
)";

    m_Shader = std::make_unique<Shader>(vs, fs);

    // Renderer global init
    Renderer::Init();

    // Initial camera aspect
    m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
}

#include <Core/Input/Input.hpp>

// ...

void EditorLayer::OnDetach()
{
    m_ActiveScene.reset();
    m_Framebuffer.reset();
    // m_CubeVA.reset();
    m_Shader.reset();
}

void EditorLayer::OnUpdate(float deltaTime)
{
    ViewportInput::UpdateCameraState(Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT));

    if (ViewportInput::IsCameraActive())
    {
        glm::vec3 dir{0.0f};

        if (Input::IsKeyPressed(GLFW_KEY_W)) dir.z += 1.0f;
        if (Input::IsKeyPressed(GLFW_KEY_S)) dir.z -= 1.0f;
        if (Input::IsKeyPressed(GLFW_KEY_A)) dir.x -= 1.0f;
        if (Input::IsKeyPressed(GLFW_KEY_D)) dir.x += 1.0f;
        if (Input::IsKeyPressed(GLFW_KEY_E)) dir.y += 1.0f;
        if (Input::IsKeyPressed(GLFW_KEY_Q)) dir.y -= 1.0f;

        double dx, dy;
        ViewportInput::GetMouseDelta(dx, dy);

        m_EditorCamera.ProcessKeyboard(dir, deltaTime);
        m_EditorCamera.ProcessMouseMovement((float)dx, (float)dy);
    }

    // Gizmo Shortcuts
    if (!ImGui::GetIO().WantTextInput)
    {
        if (Input::IsKeyPressed(GLFW_KEY_Q)) m_GizmoType = -1;
        if (Input::IsKeyPressed(GLFW_KEY_W)) m_GizmoType = ImGuizmo::TRANSLATE;
        if (Input::IsKeyPressed(GLFW_KEY_E)) m_GizmoType = ImGuizmo::ROTATE;
        if (Input::IsKeyPressed(GLFW_KEY_R)) m_GizmoType = ImGuizmo::SCALE;
    }
}

void EditorLayer::DrawThemePanel()
{
    if (!m_ShowThemePanel)
        return;

    bool open = true;
    if (!ImGui::Begin("Theme Settings", &open))
    {
        ImGui::End();
        if (!open) m_ShowThemePanel = false;
        return;
    }

    if (!open)
    {
        m_ShowThemePanel = false;
        ImGui::End();
        return;
    }

    ImGuiStyle& s = ImGui::GetStyle();

    // ---------------- TOGGLE DEFAULT THEME ----------------
    bool useDefault = ThemeSettings::UseDefaultTheme;
    if (ImGui::Checkbox("Use Default Theme", &useDefault))
    {
        ThemeSettings::UseDefaultTheme = useDefault;

        if (useDefault)
        {
            // Restore engine built-in style (not imgui default)
            s = ImGuiLayer::DefaultEngineStyle;
        }
        else
        {
            ThemeSettings::ApplyThemeFromJSON();
        }
    }

    ImGui::Separator();
    ImGui::Spacing();

    // ---------------- Rounding ----------------
    ImGui::SliderFloat("Window Rounding", &s.WindowRounding, 0.f, 20.f);
    ImGui::SliderFloat("Frame Rounding",  &s.FrameRounding,  0.f, 20.f);
    ImGui::SliderFloat("Tab Rounding",    &s.TabRounding,    0.f, 20.f);

    ImGui::Spacing(); ImGui::Separator();

    // ---------------- Padding ----------------
    ImGui::SliderFloat2("Window Padding", (float*)&s.WindowPadding, 0.f, 30.f);
    ImGui::SliderFloat2("Frame Padding",  (float*)&s.FramePadding,  0.f, 30.f);
    ImGui::SliderFloat2("Item Spacing",   (float*)&s.ItemSpacing,   0.f, 30.f);

    ImGui::Spacing(); ImGui::Separator();

    // ---------------- Colors ----------------
    ImGui::Text("Colors");
    for (int i = 0; i < ImGuiCol_COUNT; i++)
    {
        const char* name = ImGui::GetStyleColorName(i);
        ImGui::ColorEdit4(name, (float*)&s.Colors[i]);
    }

    ImGui::Spacing(); ImGui::Separator();

    // ---------------- SAVE ----------------
    if (ImGui::Button("Save Theme"))
    {
        ThemeSettings::SaveThemeToJSON();
    }

    ImGui::End();
}



void EditorLayer::OnImGuiRender()
{
    ImGuizmo::BeginFrame();

    // overriding user changes while editing in themes panel.
    // ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.0f);
    // ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);

    DrawThemePanel();
    DrawHierarchyPanel();
    DrawInspectorPanel();
    DrawContentBrowserPanel();
    DrawViewportPanel();

    // ImGui::PopStyleVar(2);
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

        // Right-click context menu to add Cube
        if (ImGui::BeginPopupContextWindow())
        {
            if (ImGui::MenuItem("Create Cube"))
            {
                SceneAPI::CreateMeshEntity(*m_ActiveScene, "Cube", Mesh::CreateCube());
            }
            ImGui::EndPopup();
        }

        if (!m_ActiveScene)
        {
            ImGui::Text("No active scene.");
            ImGui::End();
            return;
        }

        auto& reg = m_ActiveScene->Reg();
        reg.view<TagComponent>().each([&](auto entityHandle, TagComponent& tag)
            {
                Entity entity(entityHandle, m_ActiveScene.get());
                bool isSelected = (m_SelectedEntity.Handle() == entityHandle);

                ImGuiTreeNodeFlags nodeFlags =
                    ImGuiTreeNodeFlags_Leaf |
                    ImGuiTreeNodeFlags_NoTreePushOnOpen |
                    ImGuiTreeNodeFlags_SpanAvailWidth;

                if (isSelected)
                    nodeFlags |= ImGuiTreeNodeFlags_Selected;

                ImGui::TreeNodeEx((void*)(uint64_t)entityHandle, nodeFlags, "%s", tag.Tag.c_str());

                if (ImGui::IsItemClicked())
                    m_SelectedEntity = entity;
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
                tag.Tag = std::string(buffer);

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
            ImGui::DragFloat3("##Position", &transform.Position.x, 0.1f);

            ImGui::Text("Rotation");
            ImGui::SameLine();
            ImGui::DragFloat3("##Rotation", &transform.Rotation.x, 0.1f);

            ImGui::Text("Scale");
            ImGui::SameLine();
            ImGui::DragFloat3("##Scale", &transform.Scale.x, 0.1f);
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
        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };
        ImVec2 windowPos   = ImGui::GetWindowPos();
        ImVec2 cursorPos   = ImGui::GetCursorPos();     // position inside window
        ImVec2 globalImage = { windowPos.x + cursorPos.x,
                               windowPos.y + cursorPos.y };
        
        ViewportInput::SetViewportBounds(
            globalImage.x,
            globalImage.y,
            m_ViewportSize.x,
            m_ViewportSize.y
        );
        ImGui::Text("VP Bounds: X=%f Y=%f  W=%f H=%f",
            globalImage.x,
            globalImage.y,
            m_ViewportSize.x,
            m_ViewportSize.y
        );


        // if (ViewportInput::IsCameraActive())
        // {
        //     double dx, dy;
        //     ViewportInput::GetMouseDelta(dx, dy);
        //     m_EditorCamera.OnMouseMoved((float)dx, (float)dy);
        // }

        if ((uint32_t)m_ViewportSize.x > 0 &&
            (uint32_t)m_ViewportSize.y > 0 &&
            (m_Framebuffer->GetWidth() != (uint32_t)m_ViewportSize.x ||
                m_Framebuffer->GetHeight() != (uint32_t)m_ViewportSize.y))
        {
            m_Framebuffer->Resize(
                (uint32_t)m_ViewportSize.x,
                (uint32_t)m_ViewportSize.y
            );

            m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
        }

        // --- Render into framebuffer ---
        m_Framebuffer->Bind();
        Renderer::Clear({ 0.12f, 0.12f, 0.14f, 1.0f });

        // Bind shader and set view-projection matrix
        m_Shader->Bind();
        m_Shader->SetMat4("u_ViewProj", m_EditorCamera.GetViewProjection());

        // Loop through all entities with TransformComponent and MeshComponent
        auto& reg = m_ActiveScene->Reg();
        // Set default color
        m_Shader->SetFloat4("u_Color", glm::vec4(0.2f, 0.7f, 1.0f, 1.0f));

        reg.view<TransformComponent, MeshComponent>().each(
            [&](auto entity, TransformComponent& transform, MeshComponent& meshComp)
            {
                if (!meshComp.MeshHandle)
                    return;

                // Set model matrix for this entity
                glm::mat4 model = transform.GetMatrix();
                m_Shader->SetMat4("u_Model", model);

                // Bind vertex array and draw
                auto* va = meshComp.MeshHandle->GetVertexArray();
                va->Bind();

                glDrawElements(GL_TRIANGLES,
                    meshComp.MeshHandle->GetIndexCount(),
                    GL_UNSIGNED_INT,
                    nullptr);
            });
            
        // Render Selection Highlight (Wireframe)
        if (m_SelectedEntity && m_SelectedEntity.HasComponent<MeshComponent>())
        {
             auto& mc = m_SelectedEntity.GetComponent<MeshComponent>();
             if (mc.MeshHandle)
             {
                 auto& tc = m_SelectedEntity.GetComponent<TransformComponent>();
                 
                 glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                 glLineWidth(4.0f);
                 
                 m_Shader->SetFloat4("u_Color", glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)); // Orange
                 m_Shader->SetMat4("u_Model", tc.GetMatrix());
                 
                 auto* va = mc.MeshHandle->GetVertexArray();
                 va->Bind();
                 glDrawElements(GL_TRIANGLES, mc.MeshHandle->GetIndexCount(), GL_UNSIGNED_INT, nullptr);
                 
                 glLineWidth(1.0f);
                 glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
             }
        }

        m_Framebuffer->Unbind();
        // -------------------------------

        uint32_t textureID = m_Framebuffer->GetColorAttachment();
        ImGui::Image((void*)(intptr_t)textureID,
            viewportPanelSize,
            ImVec2(0, 1), ImVec2(1, 0));

        // Gizmos
        if (m_SelectedEntity && m_GizmoType != -1)
        {
            ImGuizmo::SetOrthographic(false);
            ImGuizmo::SetDrawlist();
            
            ImGuizmo::SetRect(globalImage.x, globalImage.y, m_ViewportSize.x, m_ViewportSize.y);

            // Camera
            const glm::mat4& cameraProjection = m_EditorCamera.GetProjectionMatrix();
            glm::mat4 cameraView = m_EditorCamera.GetViewMatrix();

            // Entity Transform
            auto& tc = m_SelectedEntity.GetComponent<TransformComponent>();
            glm::mat4 transform = tc.GetMatrix();

            // Snapping
            bool snap = Input::IsKeyPressed(GLFW_KEY_LEFT_CONTROL);
            float snapValue = 0.5f; // Snap to 0.5m for translation/scale
            if (m_GizmoType == ImGuizmo::ROTATE) snapValue = 45.0f; // 45 degrees
            
            float snapValues[3] = { snapValue, snapValue, snapValue };

            ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
                (ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform),
                nullptr, snap ? snapValues : nullptr);

            if (ImGuizmo::IsUsing())
            {
                float translation[3], rotation[3], scale[3];
                ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(transform), translation, rotation, scale);
                
                tc.Position = glm::vec3(translation[0], translation[1], translation[2]);
                tc.Rotation = glm::vec3(rotation[0], rotation[1], rotation[2]);
                tc.Scale = glm::vec3(scale[0], scale[1], scale[2]);
            }
        }
        
        // Mouse Picking
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && 
            ImGui::IsWindowHovered() && 
            (!m_SelectedEntity || !ImGuizmo::IsOver())) // Only check Over if we have selection (gizmo active)
        {
            ImGui::SetWindowFocus(); // Ensure shortcuts work

            ImVec2 mousePos = ImGui::GetMousePos();
            ImVec2 vpPos = globalImage;
            
            // Check inside viewport
            if (mousePos.x >= vpPos.x && mousePos.y >= vpPos.y &&
                mousePos.x < (vpPos.x + m_ViewportSize.x) && mousePos.y < (vpPos.y + m_ViewportSize.y))
            {
                // Local Mouse
                glm::vec2 localMouse = { mousePos.x - vpPos.x, mousePos.y - vpPos.y };
                
                // NDC
                float u = (localMouse.x / m_ViewportSize.x) * 2.0f - 1.0f;
                float v = ((m_ViewportSize.y - localMouse.y) / m_ViewportSize.y) * 2.0f - 1.0f;
                
                const glm::mat4& proj = m_EditorCamera.GetProjectionMatrix();
                const glm::mat4& view = m_EditorCamera.GetViewMatrix();
                // Or easier: GetViewProjection
                
                // Unproject
                glm::mat4 invVP = glm::inverse(proj * view);
                
                glm::vec4 rayStart = invVP * glm::vec4(u, v, -1.0f, 1.0f); rayStart /= rayStart.w;
                glm::vec4 rayEnd   = invVP * glm::vec4(u, v,  1.0f, 1.0f); rayEnd   /= rayEnd.w;
                
                glm::vec3 rayDir = glm::normalize(glm::vec3(rayEnd) - glm::vec3(rayStart));
                glm::vec3 rayOrigin = glm::vec3(rayStart);
                
                 // Intersect
                 m_SelectedEntity = {};
                 float minT = FLT_MAX;
                 
                 // Unit Cube AABB
                 glm::vec3 minB(-0.5f);
                 glm::vec3 maxB(0.5f);
                 
                 auto& reg = m_ActiveScene->Reg();
                 reg.view<TransformComponent, MeshComponent>().each([&](auto entity, TransformComponent& tc, MeshComponent& mc) {
                     if (!mc.MeshHandle) return;
                     
                     glm::mat4 invModel = glm::inverse(tc.GetMatrix());
                     glm::vec3 localRayOrigin = glm::vec3(invModel * glm::vec4(rayOrigin, 1.0f));
                     glm::vec3 localRayDir = glm::normalize(glm::vec3(invModel * glm::vec4(rayDir, 0.0f)));
                     
                     float t;
                     if (RayIntersectsAABB(localRayOrigin, localRayDir, minB, maxB, t))
                     {
                         if (t < minT)
                         {
                             minT = t;
                             m_SelectedEntity = { entity, m_ActiveScene.get() };
                         }
                     }
                 });
            }
        }
    }
    ImGui::End();
}
