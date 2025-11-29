#include "Window.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

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

        if (!glfwInit())
        {
            std::cout << "GLFW init failed!\n";
            return;
        }

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
            std::cout << "GLAD init failed!\n";
            return;
        }

        std::cout << "GLFW Window created: " << props.Title << "\n";
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

private:
    GLFWwindow* m_Window;

    struct WindowData
    {
        std::string Title;
        uint32_t Width, Height;
    };

    WindowData m_Data;
};

// static create function
Window* Window::Create(const WindowProps& props)
{
    return new GLFWWindow(props);
}
