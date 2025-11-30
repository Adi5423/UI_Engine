#include "VertexArray.hpp"
#include <glad/glad.h>

VertexArray::VertexArray()
{
    glGenVertexArrays(1, &m_RendererID);
}

VertexArray::~VertexArray()
{
    glDeleteVertexArrays(1, &m_RendererID);
}

void VertexArray::Bind() const
{
    glBindVertexArray(m_RendererID);
}

void VertexArray::Unbind() const
{
    glBindVertexArray(0);
}

void VertexArray::AddVertexBuffer(VertexBuffer* vb)
{
    m_VertexBuffer = vb;

    Bind();
    vb->Bind();

    // layout: position only (vec3)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,                      // index
        3,                      // vec3
        GL_FLOAT,               // type
        GL_FALSE,               // normalized
        sizeof(float) * 3,      // stride
        (void*)0                // offset
    );
}

void VertexArray::SetIndexBuffer(IndexBuffer* ib)
{
    m_IndexBuffer = ib;

    Bind();
    ib->Bind();
}
