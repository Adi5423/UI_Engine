#pragma once
#include <GLFW/glfw3.h>

class Input
{
public:
    static void Init(GLFWwindow* window);

    static bool IsKeyPressed(int key);
    static bool IsMouseButtonPressed(int button);

    static void GetMousePosition(double& x, double& y);
    static void GetMouseDelta(double& dx, double& dy);

private:
    static GLFWwindow* s_Window;
};
