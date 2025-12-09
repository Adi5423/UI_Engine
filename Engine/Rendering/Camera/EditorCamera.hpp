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
    const glm::mat4& GetViewMatrix() const { return m_View; }
    const glm::mat4& GetProjectionMatrix() const { return m_Projection; }

    // NEW — camera movement
    void ProcessKeyboard(const glm::vec3& dir, float deltaTime);
    void ProcessMouseMovement(float deltaX, float deltaY, bool constrainPitch = true);

    glm::vec3 GetPosition() const { return m_Position; }

private:
    void RecalculateView();
    void RecalculateProjection();
    void UpdateCameraVectors();

private:
    float m_FOV;
    float m_Aspect;
    float m_Near;
    float m_Far;

    // Camera transform
    glm::vec3 m_Position = { 0.0f, 0.0f, 3.0f };
    glm::vec3 m_FocalPoint = { 0.0f, 0.0f, 0.0f };

    // Direction vectors
    glm::vec3 m_Front = { 0.0f, 0.0f, -1.0f };
    glm::vec3 m_Right = { 1.0f, 0.0f, 0.0f };
    glm::vec3 m_Up = { 0.0f, 1.0f, 0.0f };

    // Euler angles
    float m_Yaw = -90.0f;
    float m_Pitch = 0.0f;

    // Movement settings
    float m_MovementSpeed = 5.0f;
    float m_MouseSensitivity = 0.1f;

    glm::mat4 m_View{ 1.0f };
    glm::mat4 m_Projection{ 1.0f };
    glm::mat4 m_ViewProjection{ 1.0f };
};
