#pragma once
#include <glm/glm.hpp>

class EditorCamera
{
public:
    EditorCamera(float fov = 45.0f,
        float aspect = 16.0f / 9.0f,
        float nearClip = 0.1f,
        float farClip = 1000.0f);

    void SetViewportSize(float width, float height);

    const glm::mat4& GetViewProjection() const { return m_ViewProjection; }

private:
    void RecalculateView();
    void RecalculateProjection();

private:
    float m_FOV;
    float m_Aspect;
    float m_Near;
    float m_Far;

    glm::vec3 m_Position = { 0.0f, 0.0f, 3.0f };
    glm::vec3 m_FocalPoint = { 0.0f, 0.0f, 0.0f };

    glm::mat4 m_View{ 1.0f };
    glm::mat4 m_Projection{ 1.0f };
    glm::mat4 m_ViewProjection{ 1.0f };
};
