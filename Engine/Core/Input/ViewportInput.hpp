#pragma once
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

class ViewportInput
{
public:
    static void Init(GLFWwindow* window);

    static void SetViewportBounds(float x, float y, float width, float height);
    static bool IsMouseInsideViewport();

    static bool IsCameraActive();  // right-click look mode
    static void UpdateCameraState(bool rightMousePressed);

    static void GetMouseDelta(double& dx, double& dy);

private:
    static GLFWwindow* s_Window;

    static float s_VP_X, s_VP_Y;
    static float s_VP_W, s_VP_H;

    static double s_LastX, s_LastY;
    static bool s_First;
    static bool s_CameraActive;
};
