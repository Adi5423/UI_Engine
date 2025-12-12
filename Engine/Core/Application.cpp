#include "Application.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "Log.hpp"

// Static singleton instance
Application* Application::s_Instance = nullptr;

Application::Application(const ApplicationSpecification& spec)
    : m_Specification(spec)
{
    // Ensure only one Application instance exists
    if (s_Instance)
    {
        CORE_ERROR("Application already exists! Only one instance allowed.");
        return;
    }
    s_Instance = this;

    // Create the window with specification
    WindowProps windowProps(
        spec.Name,
        spec.WindowWidth,
        spec.WindowHeight
    );
    m_Window = std::unique_ptr<Window>(Window::Create(windowProps));
    // Bind Event Callback
    m_Window->SetEventCallback(std::bind(&Application::OnEvent, this, std::placeholders::_1));

    CORE_INFO("Window created: {0} ({1}x{2})", spec.Name, spec.WindowWidth, spec.WindowHeight);
}

Application::~Application()
{
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
            // Update Layers
            for (Layer* layer : m_LayerStack)
                layer->OnUpdate(deltaTime);

            // Call the application update (overridden by client)
            OnUpdate(deltaTime);
        }

        // Update window (poll events -> triggers OnEvent, swap buffers)
        m_Window->OnUpdate();
    }

    // ========================================================================
    // Post-Loop Shutdown
    // ========================================================================
    OnShutdown();
}

void Application::PushLayer(Layer* layer)
{
    m_LayerStack.PushLayer(layer);
}

void Application::PushOverlay(Layer* layer)
{
    m_LayerStack.PushOverlay(layer);
}

void Application::OnEvent(EventSystem::Event& e)
{
    EventSystem::EventDispatcher dispatcher(e);
    // Dispatch window events to Application methods
    dispatcher.Dispatch<EventSystem::WindowCloseEvent>(std::bind(&Application::OnWindowClose, this, std::placeholders::_1));
    dispatcher.Dispatch<EventSystem::WindowResizeEvent>(std::bind(&Application::OnWindowResize, this, std::placeholders::_1));

    // Dispatch to Layers (Reverse order: Overlay -> Layer)
    for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
    {
        if (e.Handled) 
            break;
        (*it)->OnEvent(e);
    }
}

bool Application::OnWindowClose(EventSystem::WindowCloseEvent& e)
{
    m_Running = false;
    return true;
}

bool Application::OnWindowResize(EventSystem::WindowResizeEvent& e)
{
    if (e.GetWidth() == 0 || e.GetHeight() == 0)
    {
        m_Minimized = true;
        return false;
    }

    m_Minimized = false;
    
    // Resize viewport if necessary, though typically handled in client Or Renderer
    glViewport(0, 0, e.GetWidth(), e.GetHeight());
    
    return false;
}
