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
        Circle,
        Plane
    };

    // New 3D mesh creation API
    static std::shared_ptr<Mesh> CreateCube();
    static std::shared_ptr<Mesh> CreateTriangle3D();
    static std::shared_ptr<Mesh> CreateCircle(uint32_t segments = 32);
    static std::shared_ptr<Mesh> CreatePlane();

    VertexArray* GetVertexArray() const { return m_VertexArray.get(); }
    uint32_t GetIndexCount() const { return m_IndexCount; }
    PrimitiveType GetType() const { return m_Type; }

    const glm::vec3& GetMinAABB() const { return m_MinAABB; }
    const glm::vec3& GetMaxAABB() const { return m_MaxAABB; }

private:
    Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
    {
        m_IndexCount = (uint32_t)indices.size();

        m_VertexArray = std::make_unique<VertexArray>();

        // Create buffers with unique_ptr ownership
        auto vb = std::make_unique<VertexBuffer>(vertices.data(), vertices.size() * sizeof(Vertex));
        auto ib = std::make_unique<IndexBuffer>(indices.data(), (uint32_t)indices.size());

        m_VertexArray->Bind();
        // Transfer ownership to prevent VBO deletion (Update: AddVertexBuffer now just stores/binds, layout is set below)
        m_VertexArray->AddVertexBuffer(std::move(vb));

        // Position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));

        // Normal
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

        // Transfer ownership to VertexArray (prevents memory leak)
        m_VertexArray->SetIndexBuffer(std::move(ib));

        // Calculate AABB
        if (!vertices.empty())
        {
            m_MinAABB = vertices[0].Position;
            m_MaxAABB = vertices[0].Position;
            for (const auto& v : vertices)
            {
                m_MinAABB.x = std::min(m_MinAABB.x, v.Position.x);
                m_MinAABB.y = std::min(m_MinAABB.y, v.Position.y);
                m_MinAABB.z = std::min(m_MinAABB.z, v.Position.z);

                m_MaxAABB.x = std::max(m_MaxAABB.x, v.Position.x);
                m_MaxAABB.y = std::max(m_MaxAABB.y, v.Position.y);
                m_MaxAABB.z = std::max(m_MaxAABB.z, v.Position.z);
            }
        }
    }

private:
    std::unique_ptr<VertexArray> m_VertexArray;
    uint32_t m_IndexCount = 0;
    PrimitiveType m_Type = PrimitiveType::None;

    glm::vec3 m_MinAABB{ 0.0f };
    glm::vec3 m_MaxAABB{ 0.0f };
};
