#pragma once
#include <cstdint>

class VertexBuffer
{
public:
    VertexBuffer(const void* data, uint32_t size);
    ~VertexBuffer();

    void Bind() const;
    void Unbind() const;

    // LOW-04 FIX: GPU resources cannot be safely copied
    VertexBuffer(const VertexBuffer&) = delete;
    VertexBuffer& operator=(const VertexBuffer&) = delete;
    VertexBuffer(VertexBuffer&&) = delete;
    VertexBuffer& operator=(VertexBuffer&&) = delete;

private:
    uint32_t m_RendererID = 0;
};

class IndexBuffer
{
public:
    IndexBuffer(const uint32_t* data, uint32_t count);
    ~IndexBuffer();

    void Bind() const;
    void Unbind() const;

    // LOW-04 FIX: GPU resources cannot be safely copied
    IndexBuffer(const IndexBuffer&) = delete;
    IndexBuffer& operator=(const IndexBuffer&) = delete;
    IndexBuffer(IndexBuffer&&) = delete;
    IndexBuffer& operator=(IndexBuffer&&) = delete;

    uint32_t GetCount() const { return m_Count; }

private:
    uint32_t m_RendererID = 0;
    uint32_t m_Count = 0;
};
