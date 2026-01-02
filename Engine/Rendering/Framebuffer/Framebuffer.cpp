#include "Framebuffer.hpp"
#include <Core/GLDebug.hpp>
#include <Core/Log.hpp>
#include <Core/Application.hpp>

// Professional game engine limits (Unity/Unreal standards)
constexpr uint32_t MAX_FRAMEBUFFER_SIZE = 16384; // 16K max dimension

Framebuffer::Framebuffer(uint32_t width, uint32_t height)
    : m_Width(width), m_Height(height)
{
    Invalidate();
}

Framebuffer::~Framebuffer()
{
    GL_CALL(glDeleteFramebuffers(1, &m_RendererID));
    GL_CALL(glDeleteTextures(1, &m_ColorAttachment));
    GL_CALL(glDeleteRenderbuffers(1, &m_DepthAttachment));
}

void Framebuffer::Invalidate()
{
    if (m_RendererID)
    {
        GL_CALL(glDeleteFramebuffers(1, &m_RendererID));
        GL_CALL(glDeleteTextures(1, &m_ColorAttachment));
        GL_CALL(glDeleteRenderbuffers(1, &m_DepthAttachment));
    }

    // Create framebuffer
    GL_CALL(glGenFramebuffers(1, &m_RendererID));
    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID));

    // --- Color Texture Attachment ---
    GL_CALL(glGenTextures(1, &m_ColorAttachment));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, m_ColorAttachment));
    GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));

    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorAttachment, 0));

    // --- Depth Buffer ---
    GL_CALL(glGenRenderbuffers(1, &m_DepthAttachment));
    GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, m_DepthAttachment));
    GL_CALL(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_Width, m_Height));
    GL_CALL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_DepthAttachment));

    // Validate - CRITICAL operation, always check
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        CORE_ERROR("[Framebuffer] Incomplete! Status: 0x{0:X}", status);
    }

    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void Framebuffer::Bind()
{
    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID));
    GL_CALL(glViewport(0, 0, m_Width, m_Height));
}

void Framebuffer::Unbind()
{
    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));

    // Restore viewport to window size - cache window pointer to avoid double fetch
    Window* window = Application::Get().GetWindow();
    if (window)
    {
        uint32_t w = window->GetWidth();
        uint32_t h = window->GetHeight();
        GL_CALL(glViewport(0, 0, w, h));
    }
}

void Framebuffer::Resize(uint32_t width, uint32_t height)
{
    // Validate size bounds - prevent OOM crashes
    if (width == 0 || height == 0 || width > MAX_FRAMEBUFFER_SIZE || height > MAX_FRAMEBUFFER_SIZE)
    {
        CORE_WARN("[Framebuffer] Invalid resize request: {0}x{1} (max: {2})", 
                  width, height, MAX_FRAMEBUFFER_SIZE);
        return;
    }

    m_Width = width;
    m_Height = height;

    Invalidate();
}
