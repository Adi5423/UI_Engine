#include "Window.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "Events/ApplicationEvent.hpp"
#include "Events/KeyEvent.hpp"
#include "Events/MouseEvent.hpp"
#include "Log.hpp"

class GLFWWindow : public Window
{
public:
    GLFWWindow(const WindowProps& props)
    {
        Init(props);
    }

    virtual ~GLFWWindow()
    {
        Shutdown();
    }

    void Init(const WindowProps& props)
    {
        m_Data.Title = props.Title;
        m_Data.Width = props.Width;
        m_Data.Height = props.Height;

        CORE_INFO("Creating window {0} ({1}, {2})", props.Title, props.Width, props.Height);

        if (!glfwInit())
        {
            CORE_ERROR("Failed to initialize GLFW!");
            return;
        }

        // Error callback
        glfwSetErrorCallback([](int error, const char* description)
        {
            CORE_ERROR("GLFW Error ({0}): {1}", error, description);
        });

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        m_Window = glfwCreateWindow(
            (int)props.Width,
            (int)props.Height,
            m_Data.Title.c_str(),
            nullptr, nullptr
        );

        glfwMakeContextCurrent(m_Window);
        glfwSwapInterval(1); // vsync

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            CORE_ERROR("Failed to initialize GLAD!");
            return;
        }

        // Set User Pointer to access WindowData in callbacks
        glfwSetWindowUserPointer(m_Window, &m_Data);

        // ====================================================================
        // GLFW Event Callbacks
        // ====================================================================

        // Window Resize
        glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            data.Width = width;
            data.Height = height;

            EventSystem::WindowResizeEvent event(width, height);
            data.EventCallback(event);
        });

        // Window Close
        glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            EventSystem::WindowCloseEvent event;
            data.EventCallback(event);
        });

        // Key Callback
        glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            switch (action)
            {
                case GLFW_PRESS:
                {
                    EventSystem::KeyPressedEvent event(key, 0);
                    data.EventCallback(event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    EventSystem::KeyReleasedEvent event(key);
                    data.EventCallback(event);
                    break;
                }
                case GLFW_REPEAT:
                {
                    EventSystem::KeyPressedEvent event(key, 1);
                    data.EventCallback(event);
                    break;
                }
            }
        });

        // Mouse Button
        glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            switch (action)
            {
                case GLFW_PRESS:
                {
                    EventSystem::MouseButtonPressedEvent event(button);
                    data.EventCallback(event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    EventSystem::MouseButtonReleasedEvent event(button);
                    data.EventCallback(event);
                    break;
                }
            }
        });

        // Mouse Scroll
        glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            EventSystem::MouseScrolledEvent event((float)xOffset, (float)yOffset);
            data.EventCallback(event);
        });

        // Mouse Move
        glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            EventSystem::MouseMovedEvent event((float)xPos, (float)yPos);
            data.EventCallback(event);
        });
    }

    void Shutdown()
    {
        glfwDestroyWindow(m_Window);
        glfwTerminate();
    }

    void OnUpdate() override
    {
        glfwPollEvents();
        glfwSwapBuffers(m_Window);
    }

    uint32_t GetWidth() const override { return m_Data.Width; }
    uint32_t GetHeight() const override { return m_Data.Height; }

    void* GetNativeWindow() const override { return m_Window; }

    // Implement Event Callback Setter
    void SetEventCallback(const EventCallbackFn& callback) override
    {
        m_Data.EventCallback = callback;
    }

private:
    GLFWwindow* m_Window;

    struct WindowData
    {
        std::string Title;
        uint32_t Width, Height;
        EventCallbackFn EventCallback;
    };

    WindowData m_Data;
};

// static create function
Window* Window::Create(const WindowProps& props)
{
    return new GLFWWindow(props);
}
