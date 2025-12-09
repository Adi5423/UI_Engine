#pragma once

#include <memory>
#include <cstdint>
#include <glm/glm.hpp>

#include <Rendering/Buffers/VertexArray.hpp>

class Mesh
{
public:
    enum class PrimitiveType
    {
        None = 0,
        Cube,
        Triangle,
        Circle
    };

    // Factory methods for default primitives
    static std::shared_ptr<Mesh> CreateCube();
    static std::shared_ptr<Mesh> CreateTriangle();
    static std::shared_ptr<Mesh> CreateCircle(uint32_t segments = 32);

    VertexArray* GetVertexArray() const { return m_VertexArray.get(); }
    uint32_t GetIndexCount() const { return m_IndexCount; }
    PrimitiveType GetType() const { return m_Type; }

private:
    Mesh() = default;

    void SetVertexArray(VertexArray* va) { m_VertexArray.reset(va); }
    void SetIndexCount(uint32_t count) { m_IndexCount = count; }
    void SetType(PrimitiveType type) { m_Type = type; }

private:
    std::unique_ptr<VertexArray> m_VertexArray;
    uint32_t m_IndexCount = 0;
    PrimitiveType m_Type = PrimitiveType::None;
};
