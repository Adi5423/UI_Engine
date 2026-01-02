#include "VertexArray.hpp"
#include <glad/glad.h>

VertexArray::VertexArray()
{
    glGenVertexArrays(1, &m_RendererID);
}

VertexArray::~VertexArray()
{
    glDeleteVertexArrays(1, &m_RendererID);
    // unique_ptr automatically deletes buffers - no manual delete needed
}

void VertexArray::Bind() const
{
    glBindVertexArray(m_RendererID);
}

void VertexArray::Unbind() const
{
    glBindVertexArray(0);
}

void VertexArray::AddVertexBuffer(std::unique_ptr<VertexBuffer> vb)
{
    Bind();
    vb->Bind();
    
    m_VertexBuffer = std::move(vb); // Transfer ownership
}


void VertexArray::SetIndexBuffer(std::unique_ptr<IndexBuffer> ib)
{
    Bind();
    ib->Bind();
    
    m_IndexBuffer = std::move(ib); // Transfer ownership
}
