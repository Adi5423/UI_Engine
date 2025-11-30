#pragma once
#include <cstdint>

#include "Buffer.hpp"

class VertexArray
{
public:
    VertexArray();
    ~VertexArray();

    void Bind() const;
    void Unbind() const;

    void AddVertexBuffer(VertexBuffer* vb);
    void SetIndexBuffer(IndexBuffer* ib);

    IndexBuffer* GetIndexBuffer() const { return m_IndexBuffer; }

private:
    uint32_t m_RendererID = 0;
    VertexBuffer* m_VertexBuffer = nullptr;
    IndexBuffer* m_IndexBuffer = nullptr;
};
