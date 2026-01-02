#pragma once
#include <cstdint>
#include <memory>

#include "Buffer.hpp"

// ============================================================================
// VertexArray - Owns vertex and index buffers (RAII Pattern)
// Professional game engines use ownership semantics to prevent leaks
// ============================================================================
class VertexArray
{
public:
    VertexArray();
    ~VertexArray();

    void Bind() const;
    void Unbind() const;

    // Takes ownership of buffers - transfers ownership via unique_ptr
    void AddVertexBuffer(std::unique_ptr<VertexBuffer> vb);
    void SetIndexBuffer(std::unique_ptr<IndexBuffer> ib);

    IndexBuffer* GetIndexBuffer() const { return m_IndexBuffer.get(); }

private:
    uint32_t m_RendererID = 0;
    std::unique_ptr<VertexBuffer> m_VertexBuffer;
    std::unique_ptr<IndexBuffer> m_IndexBuffer;
};
