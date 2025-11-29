#pragma once

#include "Window.hpp"
#include "ImGuiLayer.hpp"

#include "../Editor/EditorLayer.hpp"


class Application
{
public:
    Application();
    virtual ~Application();

    void Run();

private:
    Window* m_Window;
    ImGuiLayer* m_ImGuiLayer;
    EditorLayer* m_EditorLayer;
    bool m_Running = true;

};

Application* CreateApplication();
