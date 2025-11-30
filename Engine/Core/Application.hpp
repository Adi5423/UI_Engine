#pragma once
#include "Window.hpp"

class Application
{
public:
    Application();
    virtual ~Application();

    void Run();
    void Close() { m_Running = false; }

    // Editor needs this
    Window* GetWindow() const { return m_Window; }

private:
    Window* m_Window = nullptr;
    bool m_Running = true;
};
