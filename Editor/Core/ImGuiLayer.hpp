#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include "Core/Layer.hpp"

class ImGuiLayer : public Layer
{
public:
    ImGuiLayer();
    ~ImGuiLayer();

    void OnAttach() override;
    void OnDetach() override;

    void Begin();
    void End();

    static ImGuiStyle DefaultEngineStyle;  

private:
    bool m_Enabled = false;
};
