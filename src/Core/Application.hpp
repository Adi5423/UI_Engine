#pragma once

#include "Window.hpp"
#include "ImGuiLayer.hpp"


class Application
{
public:
    Application();
    virtual ~Application();

    void Run();

private:
    Window* m_Window;
    ImGuiLayer* m_ImGuiLayer;
    bool m_Running = true;
};

Application* CreateApplication();
