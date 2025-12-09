#include "Mesh.hpp"

#include <vector>
#include <cmath>

std::shared_ptr<Mesh> Mesh::CreateCube()
{
    // Simple unit cube centered at origin
    float vertices[] = {
        // positions
        -0.5f,-0.5f,-0.5f,
         0.5f,-0.5f,-0.5f,
         0.5f, 0.5f,-0.5f,
        -0.5f, 0.5f,-0.5f,

        -0.5f,-0.5f, 0.5f,
         0.5f,-0.5f, 0.5f,
         0.5f, 0.5f, 0.5f,
        -0.5f, 0.5f, 0.5f
    };

    uint32_t indices[] = {
        0,1,2, 2,3,0, // back
        4,5,6, 6,7,4, // front
        4,5,1, 1,0,4, // bottom
        6,7,3, 3,2,6, // top
        4,0,3, 3,7,4, // left
        1,5,6, 6,2,1  // right
    };

    auto mesh = std::shared_ptr<Mesh>(new Mesh());

    auto va = new VertexArray();
    auto vb = new VertexBuffer(vertices, sizeof(vertices));
    auto ib = new IndexBuffer(indices, (uint32_t)(sizeof(indices) / sizeof(uint32_t)));

    va->AddVertexBuffer(vb);
    va->SetIndexBuffer(ib);

    mesh->SetVertexArray(va);
    mesh->SetIndexCount(ib->GetCount());
    mesh->SetType(PrimitiveType::Cube);

    return mesh;
}

std::shared_ptr<Mesh> Mesh::CreateTriangle()
{
    // Single upright triangle in XZ plane
    float vertices[] = {
        0.0f,  0.5f, 0.0f,
       -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f
    };

    uint32_t indices[] = { 0, 1, 2 };

    auto mesh = std::shared_ptr<Mesh>(new Mesh());

    auto va = new VertexArray();
    auto vb = new VertexBuffer(vertices, sizeof(vertices));
    auto ib = new IndexBuffer(indices, 3);

    va->AddVertexBuffer(vb);
    va->SetIndexBuffer(ib);

    mesh->SetVertexArray(va);
    mesh->SetIndexCount(ib->GetCount());
    mesh->SetType(PrimitiveType::Triangle);

    return mesh;
}

std::shared_ptr<Mesh> Mesh::CreateCircle(uint32_t segments)
{
    if (segments < 3)
        segments = 3;

    std::vector<float> vertices;
    std::vector<uint32_t> indices;

    // Center vertex
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);

    // Circle on XZ plane (y = 0)
    const float radius = 0.5f;
    const float twoPi = 6.28318530718f;

    for (uint32_t i = 0; i <= segments; ++i)
    {
        float theta = twoPi * (float)i / (float)segments;
        float x = radius * std::cos(theta);
        float z = radius * std::sin(theta);

        vertices.push_back(x);
        vertices.push_back(0.0f);
        vertices.push_back(z);
    }

    // Triangle fan indices (center, i, i+1)
    for (uint32_t i = 1; i <= segments; ++i)
    {
        indices.push_back(0);
        indices.push_back(i);
        indices.push_back(i + 1);
    }

    auto mesh = std::shared_ptr<Mesh>(new Mesh());

    auto va = new VertexArray();
    auto vb = new VertexBuffer(vertices.data(), (uint32_t)(vertices.size() * sizeof(float)));
    auto ib = new IndexBuffer(indices.data(), (uint32_t)indices.size());

    va->AddVertexBuffer(vb);
    va->SetIndexBuffer(ib);

    mesh->SetVertexArray(va);
    mesh->SetIndexCount(ib->GetCount());
    mesh->SetType(PrimitiveType::Circle);

    return mesh;
}
