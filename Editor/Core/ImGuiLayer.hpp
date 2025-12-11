#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>

class ImGuiLayer
{
public:
    ImGuiLayer();
    ~ImGuiLayer();

    void OnAttach(GLFWwindow* window);
    void OnDetach();

    void Begin();
    void End();

    static ImGuiStyle DefaultEngineStyle;  

private:
    bool m_Enabled = false;
};
