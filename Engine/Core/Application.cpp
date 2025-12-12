#include "Application.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

// Static singleton instance
Application* Application::s_Instance = nullptr;

Application::Application(const ApplicationSpecification& spec)
    : m_Specification(spec)
{
    // Ensure only one Application instance exists
    if (s_Instance)
    {
        std::cerr << "[ERROR] Application already exists! Only one instance allowed.\n";
        return;
    }
    s_Instance = this;

    // Create the window with specification
    WindowProps windowProps(
        spec.Name,
        spec.WindowWidth,
        spec.WindowHeight
    );
    m_Window = Window::Create(windowProps);

    std::cout << "[Engine] Window created: " << spec.Name << " (" 
              << spec.WindowWidth << "x" << spec.WindowHeight << ")\n";
}

Application::~Application()
{
    delete m_Window;
    s_Instance = nullptr;
}

void Application::Run()
{
    // ========================================================================
    // Pre-Loop Initialization
    // ========================================================================
    OnInit();

    m_LastFrameTime = (float)glfwGetTime();

    // ========================================================================
    // Main Loop
    // ========================================================================
    while (m_Running)
    {
        // Calculate delta time
        float time = (float)glfwGetTime();
        float deltaTime = time - m_LastFrameTime;
        m_LastFrameTime = time;

        // Skip rendering if minimized
        if (!m_Minimized)
        {
            // Call the application update (overridden by client)
            OnUpdate(deltaTime);
        }

        // Update window (poll events, swap buffers)
        m_Window->OnUpdate();
    }

    // ========================================================================
    // Post-Loop Shutdown
    // ========================================================================
    OnShutdown();
}
