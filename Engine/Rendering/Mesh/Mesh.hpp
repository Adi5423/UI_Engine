#pragma once

#include <glad/glad.h>
#include <memory>
#include <vector>
#include <cstdint>
#include <glm/glm.hpp>
#include <Rendering/Buffers/VertexArray.hpp>

struct Vertex
{
    glm::vec3 Position;
    glm::vec3 Normal;
};

class Mesh
{
public:
    enum class PrimitiveType
    {
        None = 0,
        Cube,
        Triangle3D,
        Circle
    };

    // New 3D mesh creation API
    static std::shared_ptr<Mesh> CreateCube();
    static std::shared_ptr<Mesh> CreateTriangle3D();
    static std::shared_ptr<Mesh> CreateCircle(uint32_t segments = 32);

    VertexArray* GetVertexArray() const { return m_VertexArray.get(); }
    uint32_t GetIndexCount() const { return m_IndexCount; }
    PrimitiveType GetType() const { return m_Type; }

private:
    Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
    {
        m_IndexCount = (uint32_t)indices.size();

        m_VertexArray = std::make_unique<VertexArray>();

        // Upload vertices
        VertexBuffer* vb = new VertexBuffer(vertices.data(), vertices.size() * sizeof(Vertex));
        IndexBuffer*  ib = new IndexBuffer(indices.data(), (uint32_t)indices.size());

        m_VertexArray->Bind();
        vb->Bind();

        // Position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));

        // Normal
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

        m_VertexArray->SetIndexBuffer(ib);
    }

private:
    std::unique_ptr<VertexArray> m_VertexArray;
    uint32_t m_IndexCount = 0;
    PrimitiveType m_Type = PrimitiveType::None;
};
