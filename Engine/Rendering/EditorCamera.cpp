#include "EditorCamera.hpp"
#include <glm/gtc/matrix_transform.hpp>

EditorCamera::EditorCamera(float fov, float aspect, float nearClip, float farClip)
    : m_FOV(fov), m_Aspect(aspect), m_Near(nearClip), m_Far(farClip)
{
    RecalculateProjection();
    RecalculateView();
}

void EditorCamera::SetViewportSize(float width, float height)
{
    if (width <= 0.0f || height <= 0.0f)
        return;

    m_Aspect = width / height;
    RecalculateProjection();
    m_ViewProjection = m_Projection * m_View;
}

void EditorCamera::RecalculateProjection()
{
    m_Projection = glm::perspective(glm::radians(m_FOV), m_Aspect, m_Near, m_Far);
}

void EditorCamera::RecalculateView()
{
    m_View = glm::lookAt(m_Position, m_FocalPoint, glm::vec3(0.0f, 1.0f, 0.0f));
    m_ViewProjection = m_Projection * m_View;
}
