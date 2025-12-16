#include "EditorLayer.hpp"
#include "EditorBridge.hpp"
#include <glm/gtc/type_ptr.hpp>

#include <cmath>

#include <imgui.h>
#include <imgui_internal.h>
#include <ImGuizmo.h>

#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <Core/Input/ViewportInput.hpp>
#include <Scene/Scene.hpp>
#include <Scene/Entity.hpp>
#include <Scene/Components.hpp>
#include <Scene/SceneAPI.hpp>
#include <Core/ThemeSettings.hpp>
#include <Core/ImGuiLayer.hpp>

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
    EditorBridge::Init(&m_CommandHistory);
    {
        m_ActiveScene = std::make_unique<Scene>();
        SceneAPI::CreateDefaultScene(*m_ActiveScene);
    }
    m_Framebuffer = std::make_unique<Framebuffer>(1280, 720);
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
    Renderer::Init();
    m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
}

// (Moved to top)

#include <Core/Input/Input.hpp>

glm::vec2 EditorLayer::WorldToScreen(const glm::vec3& worldPos, const glm::mat4& view, 
                                      const glm::mat4& proj, const glm::vec2& viewportSize, 
                                      const glm::vec2& viewportPos)
{
    // Transform to clip space
    glm::vec4 clipPos = proj * view * glm::vec4(worldPos, 1.0f);
    
    // Perspective divide
    if (clipPos.w == 0.0f) return viewportPos; // Avoid division by zero
    glm::vec3 ndcPos = glm::vec3(clipPos) / clipPos.w;
    
    // Convert to screen space
    float screenX = (ndcPos.x + 1.0f) * 0.5f * viewportSize.x + viewportPos.x;
    float screenY = (1.0f - ndcPos.y) * 0.5f * viewportSize.y + viewportPos.y;
    
    return glm::vec2(screenX, screenY);
}

void EditorLayer::OnDetach()
{
    m_ActiveScene.reset();
    m_Framebuffer.reset();
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

    if (m_ActiveScene)
        m_ActiveScene->OnUpdate(deltaTime);

    if (!ViewportInput::IsCameraActive()) // Changed else to if to allow scene update but separation logic continues
    {
        bool deletePressed = Input::IsKeyPressed(GLFW_KEY_X) || Input::IsKeyPressed(GLFW_KEY_DELETE);
        if (deletePressed && m_SelectedEntity) 
        {
             if (Input::IsKeyPressed(GLFW_KEY_X))
             {
                 if (!m_ShowDeletePopup && m_SelectedEntity.HasComponent<TransformComponent>()) 
                 {
                    m_ShowDeletePopup = true;
                    
                    // Store object's world position for later screen projection
                    auto& tc = m_SelectedEntity.GetComponent<TransformComponent>();
                    m_DeletePopupWorldPos = tc.Position;
                    m_DeletePopupNeedsPositioning = true;
                 }
             }
             else
             {
                 EditorBridge::SubmitDeleteEntity(m_SelectedEntity);
                 m_SelectedEntity = Entity();
             }
        }
    }

    if (!ImGui::GetIO().WantTextInput && !ViewportInput::IsCameraActive())
    {
        if (Input::IsKeyPressed(GLFW_KEY_Q)) m_GizmoType = -1;
        if (Input::IsKeyPressed(GLFW_KEY_W)) m_GizmoType = ImGuizmo::TRANSLATE;
        if (Input::IsKeyPressed(GLFW_KEY_E)) m_GizmoType = ImGuizmo::ROTATE;
        if (Input::IsKeyPressed(GLFW_KEY_R)) m_GizmoType = ImGuizmo::SCALE;
    }

    if (!ImGui::GetIO().WantTextInput)
    {
        bool ctrlPressed = Input::IsKeyPressed(GLFW_KEY_LEFT_CONTROL) || Input::IsKeyPressed(GLFW_KEY_RIGHT_CONTROL);
        
        if (ctrlPressed && Input::IsKeyPressed(GLFW_KEY_Z))
        {
             if (!m_UndoPressedLastFrame) 
             {
                 m_CommandHistory.Undo();
                 m_UndoPressedLastFrame = true;
             }
        }
        else
        {
             m_UndoPressedLastFrame = false;
        }
        
        if (ctrlPressed && Input::IsKeyPressed(GLFW_KEY_Y))
        {
             if (!m_RedoPressedLastFrame)
             {
                 m_CommandHistory.Redo();
                 m_RedoPressedLastFrame = true;
             }
        }
        else
        {
             m_RedoPressedLastFrame = false;
        }
    }
}

void EditorLayer::DrawThemePanel()
{
    if (!m_ShowThemePanel) return;
    bool open = true;
    if (!ImGui::Begin("Theme Settings", &open)) { ImGui::End(); if (!open) m_ShowThemePanel = false; return; }
    if (!open) { m_ShowThemePanel = false; ImGui::End(); return; }

    ImGuiStyle& s = ImGui::GetStyle();
    bool useDefault = ThemeSettings::UseDefaultTheme;
    if (ImGui::Checkbox("Use Default Theme", &useDefault))
    {
        ThemeSettings::UseDefaultTheme = useDefault;
        if (useDefault) s = ImGuiLayer::DefaultEngineStyle;
        else ThemeSettings::ApplyThemeFromJSON();
    }
    ImGui::Separator(); ImGui::Spacing();
    ImGui::SliderFloat("Window Rounding", &s.WindowRounding, 0.f, 20.f);
    ImGui::SliderFloat("Frame Rounding",  &s.FrameRounding,  0.f, 20.f);
    ImGui::SliderFloat("Tab Rounding",    &s.TabRounding,    0.f, 20.f);
    ImGui::Spacing(); ImGui::Separator();
    ImGui::SliderFloat2("Window Padding", (float*)&s.WindowPadding, 0.f, 30.f);
    ImGui::SliderFloat2("Frame Padding",  (float*)&s.FramePadding,  0.f, 30.f);
    ImGui::SliderFloat2("Item Spacing",   (float*)&s.ItemSpacing,   0.f, 30.f);
    ImGui::Spacing(); ImGui::Separator();
    ImGui::Text("Colors");
    for (int i = 0; i < ImGuiCol_COUNT; i++)
    {
        const char* name = ImGui::GetStyleColorName(i);
        ImGui::ColorEdit4(name, (float*)&s.Colors[i]);
    }
    ImGui::Spacing(); ImGui::Separator();
    if (ImGui::Button("Save Theme")) ThemeSettings::SaveThemeToJSON();
    ImGui::End();
}

void EditorLayer::OnImGuiRender()
{
    ImGuizmo::BeginFrame();
    DrawThemePanel();
    DrawHierarchyPanel();
    DrawInspectorPanel();
    DrawContentBrowserPanel();
    DrawViewportPanel();
}

void EditorLayer::DrawHierarchyPanel()
{
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus;
    if (ImGui::Begin("Hierarchy", nullptr, flags))
    {
        ImGui::TextDisabled("Scene");
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverExistingPopup))
        {
            if (ImGui::MenuItem("Create Cube"))
            {
                EditorBridge::SubmitCreateMesh(m_ActiveScene.get(), "Cube", Mesh::CreateCube());
            }
            ImGui::EndPopup();
        }

        if (!m_ActiveScene) { ImGui::Text("No active scene."); ImGui::End(); return; }

        auto& reg = m_ActiveScene->Reg();
        Entity entityToDelete;
        bool shouldDelete = false;

        reg.view<TagComponent>().each([&](auto entityHandle, TagComponent& tag)
        {
            Entity entity(entityHandle, m_ActiveScene.get());
            bool isSelected = (m_SelectedEntity.Handle() == entityHandle);
            ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanAvailWidth;
            if (isSelected) nodeFlags |= ImGuiTreeNodeFlags_Selected;

            ImGui::TreeNodeEx((void*)(uint64_t)entityHandle, nodeFlags, "%s", tag.Tag.c_str());
            if (ImGui::IsItemClicked()) m_SelectedEntity = entity;

            if (ImGui::BeginPopupContextItem())
            {
                if (ImGui::MenuItem("Delete")) { entityToDelete = entity; shouldDelete = true; }
                ImGui::EndPopup();
            }
        });

        if (shouldDelete && entityToDelete)
        {
            if (m_SelectedEntity.Handle() == entityToDelete.Handle()) m_SelectedEntity = Entity();
            EditorBridge::SubmitDeleteEntity(entityToDelete);
        }
    }
    ImGui::End();
}

void EditorLayer::DrawInspectorPanel()
{
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus;
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

        if (reg.any_of<TagComponent>(handle))
        {
            auto& tag = reg.get<TagComponent>(handle);
            char buffer[256];
            memset(buffer, 0, sizeof(buffer));
            strncpy(buffer, tag.Tag.c_str(), sizeof(buffer) - 1);

            ImGui::Text("Name");
            ImGui::SameLine();
            
            // Capture original name before modification
            std::string originalTag = tag.Tag;
            
            // Input widget
            if (ImGui::InputText("##Tag", buffer, sizeof(buffer))) tag.Tag = std::string(buffer);
            
            // On activation, store the ORIGINAL name (before any potential immediate edit)
            if (ImGui::IsItemActivated())
            {
                m_PreviousName = originalTag;
            }
            
            if (ImGui::IsItemDeactivatedAfterEdit() || (ImGui::IsItemActive() && Input::IsKeyPressed(GLFW_KEY_ENTER)))
            {
                std::string newName = tag.Tag;
                if (newName != m_PreviousName)
                {
                    EditorBridge::SubmitRename(m_SelectedEntity, m_PreviousName, newName);
                    m_PreviousName = newName; 
                }
            }
            ImGui::Separator();
        }

        if (reg.any_of<TransformComponent>(handle))
        {
            auto& transform = reg.get<TransformComponent>(handle);
            ImGui::Text("Transform");
            ImGui::Separator();

            ImGui::Text("Position");
            ImGui::SameLine();
            ImGui::DragFloat3("##Position", &transform.Position.x, 0.1f);
            
            // Detect when user starts editing
            if (ImGui::IsItemActivated())
            {
                m_TransformEditState.savedTransform = transform;
                m_TransformEditState.isEditingPosition = true;
            }
            
            if (m_TransformEditState.isEditingPosition)
            {
                if (ImGui::IsItemDeactivatedAfterEdit() || (ImGui::IsItemActive() && Input::IsKeyPressed(GLFW_KEY_ENTER)))
                {
                    EditorBridge::SubmitTransformChange(m_SelectedEntity, m_TransformEditState.savedTransform, transform);
                    m_TransformEditState.isEditingPosition = false;
                }
            }

            ImGui::Text("Rotation");
            ImGui::SameLine();
            ImGui::DragFloat3("##Rotation", &transform.Rotation.x, 0.1f);
            
            if (ImGui::IsItemActivated())
            {
                m_TransformEditState.savedTransform = transform;
                m_TransformEditState.isEditingRotation = true;
            }
            
            if (m_TransformEditState.isEditingRotation)
            {
                if (ImGui::IsItemDeactivatedAfterEdit() || (ImGui::IsItemActive() && Input::IsKeyPressed(GLFW_KEY_ENTER)))
                {
                    EditorBridge::SubmitTransformChange(m_SelectedEntity, m_TransformEditState.savedTransform, transform);
                    m_TransformEditState.isEditingRotation = false;
                }
            }

            ImGui::Text("Scale");
            ImGui::SameLine();
            ImGui::DragFloat3("##Scale", &transform.Scale.x, 0.1f);
            
            if (ImGui::IsItemActivated())
            {
                m_TransformEditState.savedTransform = transform;
                m_TransformEditState.isEditingScale = true;
            }
            
            if (m_TransformEditState.isEditingScale)
            {
                if (ImGui::IsItemDeactivatedAfterEdit() || (ImGui::IsItemActive() && Input::IsKeyPressed(GLFW_KEY_ENTER)))
                {
                    EditorBridge::SubmitTransformChange(m_SelectedEntity, m_TransformEditState.savedTransform, transform);
                    m_TransformEditState.isEditingScale = false;
                }
            }
        }
    }
    ImGui::End();
}

void EditorLayer::DrawContentBrowserPanel()
{
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus;
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
    ImGuiWindowFlags viewportFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
    if (ImGui::Begin("Viewport", nullptr, viewportFlags))
    {
        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };
        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 cursorPos = ImGui::GetCursorPos();
        ImVec2 globalImage = { windowPos.x + cursorPos.x, windowPos.y + cursorPos.y };
        
        ViewportInput::SetViewportBounds(globalImage.x, globalImage.y, m_ViewportSize.x, m_ViewportSize.y);
        ImGui::Text("VP Bounds: X=%f Y=%f  W=%f H=%f", globalImage.x, globalImage.y, m_ViewportSize.x, m_ViewportSize.y);

        if ((uint32_t)m_ViewportSize.x > 0 && (uint32_t)m_ViewportSize.y > 0 &&
            (m_Framebuffer->GetWidth() != (uint32_t)m_ViewportSize.x || m_Framebuffer->GetHeight() != (uint32_t)m_ViewportSize.y))
        {
            m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
            m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
        }

        // Calculate delete popup screen position if needed
        if (m_DeletePopupNeedsPositioning && m_SelectedEntity && m_SelectedEntity.HasComponent<TransformComponent>())
        {
            auto& tc = m_SelectedEntity.GetComponent<TransformComponent>();
            m_DeletePopupPos = WorldToScreen(
                tc.Position,
                m_EditorCamera.GetViewMatrix(),
                m_EditorCamera.GetProjectionMatrix(),
                m_ViewportSize,
                glm::vec2(globalImage.x, globalImage.y)
            );
            
            // Clamp to viewport bounds with padding
            float padding = 10.0f;
            float popupWidth = 150.0f;  // Approximate popup width
            float popupHeight = 80.0f;  // Approximate popup height
            
            m_DeletePopupPos.x = glm::clamp(m_DeletePopupPos.x, 
                                            globalImage.x + padding, 
                                            globalImage.x + m_ViewportSize.x - popupWidth - padding);
            m_DeletePopupPos.y = glm::clamp(m_DeletePopupPos.y, 
                                            globalImage.y + padding, 
                                            globalImage.y + m_ViewportSize.y - popupHeight - padding);
            
            m_DeletePopupNeedsPositioning = false;
        }

        m_Framebuffer->Bind();
        glEnable(GL_DEPTH_TEST); // Ensure depth test is on for 3D rendering
        Renderer::Clear({ 0.12f, 0.12f, 0.14f, 1.0f });
        m_Shader->Bind();
        m_Shader->SetMat4("u_ViewProj", m_EditorCamera.GetViewProjection());

        auto& reg = m_ActiveScene->Reg();
        m_Shader->SetFloat4("u_Color", glm::vec4(0.2f, 0.7f, 1.0f, 1.0f));
        reg.view<TransformComponent, MeshComponent>().each([&](auto entity, TransformComponent& transform, MeshComponent& meshComp)
        {
            if (!meshComp.MeshHandle) return;
            if (m_GizmoPreviewActive && m_SelectedEntity && m_GizmoPreviewEntity == (entt::entity)entity)
                m_Shader->SetMat4("u_Model", m_GizmoPreviewMatrix);
            else
                m_Shader->SetMat4("u_Model", transform.GetMatrix());
            auto* va = meshComp.MeshHandle->GetVertexArray();
            va->Bind();
            glDrawElements(GL_TRIANGLES, meshComp.MeshHandle->GetIndexCount(), GL_UNSIGNED_INT, nullptr);
        });
            
        if (m_SelectedEntity && m_SelectedEntity.HasComponent<MeshComponent>())
        {
             auto& mc = m_SelectedEntity.GetComponent<MeshComponent>();
             if (mc.MeshHandle)
             {
                 auto& tc = m_SelectedEntity.GetComponent<TransformComponent>();
                 glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                 glLineWidth(4.0f);
                 m_Shader->SetFloat4("u_Color", glm::vec4(1.0f, 0.5f, 0.0f, 1.0f));
                 if (m_GizmoPreviewActive)
                     m_Shader->SetMat4("u_Model", m_GizmoPreviewMatrix);
                 else
                     m_Shader->SetMat4("u_Model", tc.GetMatrix());
                 auto* va = mc.MeshHandle->GetVertexArray();
                 va->Bind();
                 glDrawElements(GL_TRIANGLES, mc.MeshHandle->GetIndexCount(), GL_UNSIGNED_INT, nullptr);
                 glLineWidth(1.0f);
                 glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
             }
        }

        m_Framebuffer->Unbind();
        uint32_t textureID = m_Framebuffer->GetColorAttachment();
        ImGui::Image((void*)(intptr_t)textureID, viewportPanelSize, ImVec2(0, 1), ImVec2(1, 0));

        if (m_SelectedEntity && m_GizmoType != -1)
        {
            ImGuizmo::SetOrthographic(false);
            ImGuizmo::SetDrawlist();
            ImGuizmo::SetRect(globalImage.x, globalImage.y, m_ViewportSize.x, m_ViewportSize.y);
            const glm::mat4& cameraProjection = m_EditorCamera.GetProjectionMatrix();
            glm::mat4 cameraView = m_EditorCamera.GetViewMatrix();
            auto& tc = m_SelectedEntity.GetComponent<TransformComponent>();
            if (!m_GizmoPreviewActive || m_GizmoPreviewEntity != m_SelectedEntity.Handle())
            {
                m_GizmoPreviewActive = false;
                m_GizmoPreviewEntity = entt::null;
                m_GizmoPreviewMatrix = tc.GetMatrix();
            }

            glm::mat4 transform = m_GizmoPreviewActive ? m_GizmoPreviewMatrix : tc.GetMatrix();
            glm::mat4 deltaMatrix(1.0f);

            bool snap = Input::IsKeyPressed(GLFW_KEY_LEFT_CONTROL);
            float snapValue = 0.5f;
            if (m_GizmoType == ImGuizmo::ROTATE) snapValue = 45.0f;
            float snapValues[3] = { snapValue, snapValue, snapValue };

            ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
                (ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform),
                glm::value_ptr(deltaMatrix), snap ? snapValues : nullptr);

            if (ImGuizmo::IsUsing())
            {
                if (!m_WasUsingGizmo)
                {
                    m_TransformEditState.savedTransform = tc;
                    m_WasUsingGizmo = true;
                    m_GizmoTypeAtStart = m_GizmoType;
                    m_GizmoPreviewActive = true;
                    m_GizmoPreviewEntity = m_SelectedEntity.Handle();
                    m_GizmoPreviewMatrix = tc.GetMatrix();
                }

                // Keep a live matrix for rendering while dragging.
                m_GizmoPreviewActive = true;
                m_GizmoPreviewEntity = m_SelectedEntity.Handle();
                m_GizmoPreviewMatrix = transform;
            }
            else if (m_WasUsingGizmo)
            {
                // Commit once (avoid per-frame matrix->euler feedback issues)
                // Use GLM decomposition for consistency with TransformComponent's new quaternion logic
                glm::vec3 scale;
                glm::quat rotation;
                glm::vec3 translation;
                glm::vec3 skew;
                glm::vec4 perspective;
                glm::decompose(transform, scale, rotation, translation, skew, perspective);

                tc.Position = translation;
                tc.Rotation = glm::degrees(glm::eulerAngles(rotation));
                tc.Scale = scale;

                m_GizmoPreviewActive = false;
                m_GizmoPreviewEntity = entt::null;
                m_GizmoTypeAtStart = -1;

                EditorBridge::SubmitTransformChange(m_SelectedEntity, m_TransformEditState.savedTransform, tc);
                m_WasUsingGizmo = false;
            }
        }
        
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered() && (!m_SelectedEntity || !ImGuizmo::IsOver()) && !ImGui::IsPopupOpen("DeleteConfirmation"))
        {
            ImGui::SetWindowFocus();
            ImVec2 mousePos = ImGui::GetMousePos();
            ImVec2 vpPos = globalImage;
            if (mousePos.x >= vpPos.x && mousePos.y >= vpPos.y && mousePos.x < (vpPos.x + m_ViewportSize.x) && mousePos.y < (vpPos.y + m_ViewportSize.y))
            {
                glm::vec2 localMouse = { mousePos.x - vpPos.x, mousePos.y - vpPos.y };
                float u = (localMouse.x / m_ViewportSize.x) * 2.0f - 1.0f;
                float v = ((m_ViewportSize.y - localMouse.y) / m_ViewportSize.y) * 2.0f - 1.0f;
                const glm::mat4& proj = m_EditorCamera.GetProjectionMatrix();
                const glm::mat4& view = m_EditorCamera.GetViewMatrix();
                glm::mat4 invVP = glm::inverse(proj * view);
                glm::vec4 rayStart = invVP * glm::vec4(u, v, -1.0f, 1.0f); rayStart /= rayStart.w;
                glm::vec4 rayEnd   = invVP * glm::vec4(u, v,  1.0f, 1.0f); rayEnd   /= rayEnd.w;
                glm::vec3 rayDir = glm::normalize(glm::vec3(rayEnd) - glm::vec3(rayStart));
                glm::vec3 rayOrigin = glm::vec3(rayStart);
                
                 m_SelectedEntity = {};
                 float minT = FLT_MAX;
                 auto& reg = m_ActiveScene->Reg();
                 reg.view<TransformComponent, MeshComponent>().each([&](auto entity, TransformComponent& tc, MeshComponent& mc) {
                     if (!mc.MeshHandle) return;
                     
                     glm::vec3 minB = mc.MeshHandle->GetMinAABB();
                     glm::vec3 maxB = mc.MeshHandle->GetMaxAABB();

                     glm::mat4 invModel = glm::inverse(tc.GetMatrix());
                     glm::vec3 localRayOrigin = glm::vec3(invModel * glm::vec4(rayOrigin, 1.0f));
                     glm::vec3 localRayDir = glm::normalize(glm::vec3(invModel * glm::vec4(rayDir, 0.0f)));
                     float t;
                     if (RayIntersectsAABB(localRayOrigin, localRayDir, minB, maxB, t))
                     {
                         if (t < minT && t >= 0.0f)
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
    
    // Delete Popup Logic
    if (m_ShowDeletePopup)
    {
        ImGui::OpenPopup("DeleteConfirmation");
        m_ShowDeletePopup = false; // Request consumed
    }

    ImGui::SetNextWindowPos(ImVec2(m_DeletePopupPos.x, m_DeletePopupPos.y), ImGuiCond_Appearing);
    if (ImGui::BeginPopup("DeleteConfirmation"))
    {
        ImGui::Text("Delete Selected?");
        ImGui::Spacing();
        
        if (ImGui::Button("OK", ImVec2(120, 0)) || Input::IsKeyPressed(GLFW_KEY_ENTER)) 
        {
            if (m_SelectedEntity)
            {
                EditorBridge::SubmitDeleteEntity(m_SelectedEntity);
                m_SelectedEntity = Entity();
            }
            ImGui::CloseCurrentPopup();
        }
        
        if (Input::IsKeyPressed(GLFW_KEY_ESCAPE))
        {
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
    }
}
