#pragma once
#include <string>
#include <functional>
#include <cstdint>   // for uint32_t

struct WindowProps
{
    std::string Title;
    uint32_t Width;
    uint32_t Height;

    WindowProps(const std::string& title = "Groove Engine Pre builds",
        uint32_t width = 1280,
        uint32_t height = 720)
        : Title(title), Width(width), Height(height) {
    }
};

class Window
{
public:
    virtual ~Window() {}

    virtual void OnUpdate() = 0;

    virtual uint32_t GetWidth() const = 0;
    virtual uint32_t GetHeight() const = 0;

    virtual void* GetNativeWindow() const = 0;

    static Window* Create(const WindowProps& props = WindowProps());
};
