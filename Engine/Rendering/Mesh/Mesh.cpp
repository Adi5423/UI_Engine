#include "Mesh.hpp"
#include <vector>
#include <cmath>

//
// ---------- 3D CUBE ----------
//
std::shared_ptr<Mesh> Mesh::CreateCube()
{
    std::vector<Vertex> vertices =
    {
        // FRONT
        {{-0.5f,-0.5f, 0.5f}, {0,0,1}},
        {{ 0.5f,-0.5f, 0.5f}, {0,0,1}},
        {{ 0.5f, 0.5f, 0.5f}, {0,0,1}},
        {{-0.5f, 0.5f, 0.5f}, {0,0,1}},

        // BACK
        {{-0.5f,-0.5f,-0.5f}, {0,0,-1}},
        {{ 0.5f,-0.5f,-0.5f}, {0,0,-1}},
        {{ 0.5f, 0.5f,-0.5f}, {0,0,-1}},
        {{-0.5f, 0.5f,-0.5f}, {0,0,-1}},

        // LEFT
        {{-0.5f,-0.5f,-0.5f}, {-1,0,0}},
        {{-0.5f,-0.5f, 0.5f}, {-1,0,0}},
        {{-0.5f, 0.5f, 0.5f}, {-1,0,0}},
        {{-0.5f, 0.5f,-0.5f}, {-1,0,0}},

        // RIGHT
        {{ 0.5f,-0.5f,-0.5f}, {1,0,0}},
        {{ 0.5f,-0.5f, 0.5f}, {1,0,0}},
        {{ 0.5f, 0.5f, 0.5f}, {1,0,0}},
        {{ 0.5f, 0.5f,-0.5f}, {1,0,0}},

        // TOP
        {{-0.5f, 0.5f, 0.5f}, {0,1,0}},
        {{ 0.5f, 0.5f, 0.5f}, {0,1,0}},
        {{ 0.5f, 0.5f,-0.5f}, {0,1,0}},
        {{-0.5f, 0.5f,-0.5f}, {0,1,0}},

        // BOTTOM
        {{-0.5f,-0.5f, 0.5f}, {0,-1,0}},
        {{ 0.5f,-0.5f, 0.5f}, {0,-1,0}},
        {{ 0.5f,-0.5f,-0.5f}, {0,-1,0}},
        {{-0.5f,-0.5f,-0.5f}, {0,-1,0}},
    };

    std::vector<uint32_t> indices =
    {
        // FRONT
         0, 1, 2,   2, 3, 0,
        // BACK
         4, 5, 6,   6, 7, 4,
        // LEFT
         8, 9,10,  10,11, 8,
        // RIGHT
        12,13,14,  14,15,12,
        // TOP
        16,17,18,  18,19,16,
        // BOTTOM
        20,21,22,  22,23,20
    };

    auto mesh = std::shared_ptr<Mesh>(new Mesh(vertices, indices));
    mesh->m_Type = PrimitiveType::Cube;
    return mesh;
}

//
// ---------- 3D PYRAMID (Triangle3D) ----------
//
std::shared_ptr<Mesh> Mesh::CreateTriangle3D()
{
    std::vector<Vertex> vertices =
    {
        // Base square
        {{-0.5f,0.0f,-0.5f}, {0,-1,0}},
        {{ 0.5f,0.0f,-0.5f}, {0,-1,0}},
        {{ 0.5f,0.0f, 0.5f}, {0,-1,0}},
        {{-0.5f,0.0f, 0.5f}, {0,-1,0}},

        // Tip
        {{0.0f,1.0f,0.0f}, {0,1,0}},
    };

    std::vector<uint32_t> indices =
    {
        // Base
        0,1,2, 2,3,0,

        // Sides
        0,1,4,
        1,2,4,
        2,3,4,
        3,0,4
    };

    auto mesh = std::shared_ptr<Mesh>(new Mesh(vertices, indices));
    mesh->m_Type = PrimitiveType::Triangle3D;
    return mesh;
}

//
// ---------- CIRCLE (Flat Disc) ----------
//
std::shared_ptr<Mesh> Mesh::CreateCircle(uint32_t segments)
{
    if (segments < 3) segments = 3;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    // Center
    vertices.push_back({ {0,0,0}, {0,1,0} });

    float radius = 0.5f;
    float step = 6.28318530718f / segments;

    for (uint32_t i = 0; i <= segments; ++i)
    {
        float t = step * i;
        float x = std::cos(t) * radius;
        float z = std::sin(t) * radius;
        vertices.push_back({ {x,0,z}, {0,1,0} });
    }

    for (uint32_t i = 1; i <= segments; i++)
    {
        indices.push_back(0);
        indices.push_back(i);
        indices.push_back(i + 1);
    }

    auto mesh = std::shared_ptr<Mesh>(new Mesh(vertices, indices));
    mesh->m_Type = PrimitiveType::Circle;
    return mesh;
}
