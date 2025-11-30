#pragma once

#include <stdint.h>
#include <glad/glad.h>

class Framebuffer
{
public:
    Framebuffer(uint32_t width, uint32_t height);
    ~Framebuffer();

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

    uint32_t m_Width, m_Height;
};
