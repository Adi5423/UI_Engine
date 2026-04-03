#pragma once

#include <stdint.h>
#include <glad/glad.h>

class Framebuffer
{
public:
    Framebuffer(uint32_t width, uint32_t height);
    ~Framebuffer();

    // LOW-04 FIX: GPU resources cannot be safely copied
    Framebuffer(const Framebuffer&) = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;
    Framebuffer(Framebuffer&&) = delete;
    Framebuffer& operator=(Framebuffer&&) = delete;

    void Bind();
    void Unbind();

    void Resize(uint32_t width, uint32_t height);

    uint32_t GetColorAttachment() const { return m_ColorAttachment; }
    uint32_t GetWidth() const { return m_Width; }
    uint32_t GetHeight() const { return m_Height; }

private:
    void Invalidate();

private:
    uint32_t m_RendererID = 0;
    uint32_t m_ColorAttachment = 0;
    uint32_t m_DepthAttachment = 0;

    // BUG-003 FIX: Initialize to prevent undefined behavior
    uint32_t m_Width = 0, m_Height = 0;
};
