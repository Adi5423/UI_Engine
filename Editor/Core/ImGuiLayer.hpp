#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include "ThemeSettings.hpp"


class ImGuiLayer
{
public:
    ImGuiLayer();
    ~ImGuiLayer();

    void OnAttach(GLFWwindow* window);
    void OnDetach();

    void Begin();
    void End();

private:
    bool m_Enabled = false;
};
