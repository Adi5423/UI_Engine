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
    
    // BUG-016 FIX: Append to vector instead of replacing
    m_VertexBuffers.push_back(std::move(vb));
}


void VertexArray::SetIndexBuffer(std::unique_ptr<IndexBuffer> ib)
{
    Bind();
    ib->Bind();
    
    m_IndexBuffer = std::move(ib); // Transfer ownership
    
    // BUG-006 FIX: Unbind VAO to prevent state pollution
    Unbind();
}
