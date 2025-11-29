#include "Application.hpp"
#include <glad/glad.h>
#include <iostream>

Application::Application()
{
    m_Window = Window::Create();
}

Application::~Application()
{
    delete m_Window;
}

void Application::Run()
{
    while (m_Running)
    {
        // Clear screen
        glClearColor(0.1f, 0.1f, 0.1f, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        m_Window->OnUpdate();
    }
}
