#include "EditorCamera.hpp"
#include <glm/gtc/matrix_transform.hpp>

EditorCamera::EditorCamera(float fov, float aspect, float nearClip, float farClip)
    : m_FOV(fov), m_Aspect(aspect), m_Near(nearClip), m_Far(farClip)
{
    RecalculateProjection();
    UpdateCameraVectors();
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

// ---------------------------------------------------
// NEW: Movement controls
// ---------------------------------------------------
void EditorCamera::ProcessKeyboard(const glm::vec3& dir, float deltaTime)
{
    float velocity = m_MovementSpeed * deltaTime;
    m_Position += m_Front * dir.z * velocity;
    m_Position += m_Right * dir.x * velocity;
    m_Position += m_Up * dir.y * velocity;
    RecalculateView();
}

void EditorCamera::ProcessMouseMovement(float deltaX, float deltaY, bool constrainPitch)
{
    deltaX *= m_MouseSensitivity;
    deltaY *= m_MouseSensitivity;

    m_Yaw += deltaX;
    m_Pitch += deltaY;

    if (constrainPitch)
    {
        if (m_Pitch > 89.0f)  m_Pitch = 89.0f;
        if (m_Pitch < -89.0f) m_Pitch = -89.0f;
    }

    UpdateCameraVectors();
    RecalculateView();
}

// ---------------------------------------------------

void EditorCamera::RecalculateProjection()
{
    m_Projection = glm::perspective(glm::radians(m_FOV), m_Aspect, m_Near, m_Far);
}

void EditorCamera::RecalculateView()
{
    m_View = glm::lookAt(m_Position, m_Position + m_Front, m_Up);
    m_ViewProjection = m_Projection * m_View;
}

void EditorCamera::UpdateCameraVectors()
{
    glm::vec3 front;
    front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    front.y = sin(glm::radians(m_Pitch));
    front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));

    m_Front = glm::normalize(front);
    m_Right = glm::normalize(glm::cross(m_Front, {0.0f, 1.0f, 0.0f}));
    m_Up    = glm::normalize(glm::cross(m_Right, m_Front));
}
