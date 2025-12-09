#include "ViewportInput.hpp"

GLFWwindow* ViewportInput::s_Window = nullptr;

float ViewportInput::s_VP_X = 0.0f;
float ViewportInput::s_VP_Y = 0.0f;
float ViewportInput::s_VP_W = 0.0f;
float ViewportInput::s_VP_H = 0.0f;

double ViewportInput::s_LastX = 0.0;
double ViewportInput::s_LastY = 0.0;
bool ViewportInput::s_First = true;
bool ViewportInput::s_CameraActive = false;

void ViewportInput::Init(GLFWwindow* window)
{
    s_Window = window;
}

void ViewportInput::SetViewportBounds(float x, float y, float width, float height)
{
    s_VP_X = x;
    s_VP_Y = y;
    s_VP_W = width;
    s_VP_H = height;
}

bool ViewportInput::IsMouseInsideViewport()
{
    double mx, my;
    glfwGetCursorPos(s_Window, &mx, &my);

    return (mx >= s_VP_X && mx <= s_VP_X + s_VP_W &&
            my >= s_VP_Y && my <= s_VP_Y + s_VP_H);
}

bool ViewportInput::IsCameraActive()
{
    return s_CameraActive;
}

void ViewportInput::UpdateCameraState(bool rightMousePressed)
{
    bool inside = IsMouseInsideViewport();

    // --- ENTER CAMERA MODE (one time) ---
    if (!s_CameraActive && rightMousePressed && inside)
    {
        s_CameraActive = true;
        s_First = true;   // reset delta
        glfwSetInputMode(s_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        return;
    }

    // --- EXIT CAMERA MODE ---
    if (s_CameraActive && !rightMousePressed)
    {
        s_CameraActive = false;
        glfwSetInputMode(s_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        return;
    }

    // --- KEEP CAMERA MODE ACTIVE ---
    // do NOT switch cursor modes here (prevents flicker)
}

void ViewportInput::GetMouseDelta(double& dx, double& dy)
{
    dx = dy = 0.0;
    if (!s_CameraActive) return;

    double x, y;
    glfwGetCursorPos(s_Window, &x, &y);

    if (s_First)
    {
        s_LastX = x;
        s_LastY = y;
        s_First = false;
    }

    dx = x - s_LastX;
    dy = s_LastY - y;

    s_LastX = x;
    s_LastY = y;
}
