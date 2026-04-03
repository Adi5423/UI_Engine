#include "Window.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "Events/ApplicationEvent.hpp"
#include "Events/KeyEvent.hpp"
#include "Events/MouseEvent.hpp"
#include "Log.hpp"

static bool s_GLFWInitialized = false;

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

    static void GLFWErrorCallback(int error, const char* description)
    {
        CORE_ERROR("GLFW Error (", error, "): ", description);
    }

    void Init(const WindowProps& props)
    {
        m_Data.Title = props.Title;
        m_Data.Width = props.Width;
        m_Data.Height = props.Height;

        CORE_INFO("Creating window ", props.Title, " (", props.Width, ", ", props.Height, ")");

        if (!s_GLFWInitialized)
        {
            int success = glfwInit();
            if (success)
            {
                s_GLFWInitialized = true;
                glfwSetErrorCallback(GLFWErrorCallback);
            }
            else
            {
                CORE_ERROR("Failed to initialize GLFW!");
                return;
            }
        }

#ifdef __APPLE__
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required for macOS
        glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE); // Support Retina displays
#else
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
#endif
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE); // Launch maximized

        m_Window = glfwCreateWindow(
            (int)props.Width,
            (int)props.Height,
            m_Data.Title.c_str(),
            nullptr, nullptr
        );

        if (!m_Window)
        {
            CORE_ERROR("Failed to create GLFW window!");
            glfwTerminate();
            return;
        }

        glfwMakeContextCurrent(m_Window);
        glfwSwapInterval(1); // vsync

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            CORE_ERROR("Failed to initialize GLAD!");
            return;
        }

        // BUG-027 FIX: Validate OpenGL version
        int major, minor;
        glGetIntegerv(GL_MAJOR_VERSION, &major);
        glGetIntegerv(GL_MINOR_VERSION, &minor);
        CORE_INFO("OpenGL Version: ", major, ".", minor);
        
        #ifdef __APPLE__
            if (major < 4 || (major == 4 && minor < 1))
            {
                CORE_ERROR("OpenGL 4.1 or higher required! Got {}.{}", major, minor);
            }
        #else
            if (major < 4 || (major == 4 && minor < 5))
            {
                CORE_WARN("OpenGL 4.5 recommended, got {}.{}", major, minor);
            }
        #endif

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
    GLFWwindow* m_Window = nullptr;  // CRIT-05 FIX: Prevent garbage pointer on init failure

    struct WindowData
    {
        std::string Title;
        uint32_t Width = 0, Height = 0;
        // CRIT-06 FIX: Default no-op callback prevents crash if events fire before SetEventCallback
        EventCallbackFn EventCallback = [](EventSystem::Event&) {};
    };

    WindowData m_Data;
};

// static create function
Window* Window::Create(const WindowProps& props)
{
    // CRIT-05 FIX: Validate initialization succeeded before returning
    GLFWWindow* window = new GLFWWindow(props);
    if (!window->GetNativeWindow())
    {
        CORE_ERROR("Failed to create Window! Terminating.");
        delete window;
        return nullptr;
    }
    return window;
}
