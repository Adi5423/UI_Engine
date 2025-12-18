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
        // ----------------------------------------------------
        // FRONT FACE (Z = +0.5, normal = +Z)
        // CCW when looking from front
        // ----------------------------------------------------
        {{-0.5f, -0.5f,  0.5f}, {0, 0, 1}}, // 0 bottom-left
        {{ 0.5f, -0.5f,  0.5f}, {0, 0, 1}}, // 1 bottom-right
        {{ 0.5f,  0.5f,  0.5f}, {0, 0, 1}}, // 2 top-right
        {{-0.5f,  0.5f,  0.5f}, {0, 0, 1}}, // 3 top-left

        // ----------------------------------------------------
        // BACK FACE (Z = -0.5, normal = -Z)
        // CCW when looking from back
        // ----------------------------------------------------
        {{ 0.5f, -0.5f, -0.5f}, {0, 0,-1}}, // 4 bottom-right
        {{-0.5f, -0.5f, -0.5f}, {0, 0,-1}}, // 5 bottom-left
        {{-0.5f,  0.5f, -0.5f}, {0, 0,-1}}, // 6 top-left
        {{ 0.5f,  0.5f, -0.5f}, {0, 0,-1}}, // 7 top-right

        // ----------------------------------------------------
        // RIGHT FACE (X = +0.5, normal = +X)
        // CCW when looking from right
        // ----------------------------------------------------
        {{ 0.5f, -0.5f,  0.5f}, {1, 0, 0}}, // 8  bottom-front
        {{ 0.5f, -0.5f, -0.5f}, {1, 0, 0}}, // 9  bottom-back
        {{ 0.5f,  0.5f, -0.5f}, {1, 0, 0}}, // 10 top-back
        {{ 0.5f,  0.5f,  0.5f}, {1, 0, 0}}, // 11 top-front

        // ----------------------------------------------------
        // LEFT FACE (X = -0.5, normal = -X)
        // CCW when looking from left
        // ----------------------------------------------------
        {{-0.5f, -0.5f, -0.5f}, {-1, 0, 0}}, // 12 bottom-back
        {{-0.5f, -0.5f,  0.5f}, {-1, 0, 0}}, // 13 bottom-front
        {{-0.5f,  0.5f,  0.5f}, {-1, 0, 0}}, // 14 top-front
        {{-0.5f,  0.5f, -0.5f}, {-1, 0, 0}}, // 15 top-back

        // ----------------------------------------------------
        // TOP FACE (Y = +0.5, normal = +Y)
        // CCW when looking from top
        // ----------------------------------------------------
        {{-0.5f,  0.5f,  0.5f}, {0, 1, 0}}, // 16 front-left
        {{ 0.5f,  0.5f,  0.5f}, {0, 1, 0}}, // 17 front-right
        {{ 0.5f,  0.5f, -0.5f}, {0, 1, 0}}, // 18 back-right
        {{-0.5f,  0.5f, -0.5f}, {0, 1, 0}}, // 19 back-left

        // ----------------------------------------------------
        // BOTTOM FACE (Y = -0.5, normal = -Y)
        // CCW when looking from bottom
        // ----------------------------------------------------
        {{-0.5f, -0.5f, -0.5f}, {0,-1, 0}}, // 20 back-left
        {{ 0.5f, -0.5f, -0.5f}, {0,-1, 0}}, // 21 back-right
        {{ 0.5f, -0.5f,  0.5f}, {0,-1, 0}}, // 22 front-right
        {{-0.5f, -0.5f,  0.5f}, {0,-1, 0}}  // 23 front-left
    };

    std::vector<uint32_t> indices =
    {
        // FRONT
        0,1,2,  2,3,0,
    
        // BACK (FIXED)
        5,4,7,  7,6,5,
    
        // RIGHT
        8,9,10, 10,11,8,
    
        // LEFT
        12,13,14, 14,15,12,
    
        // TOP
        16,17,18, 18,19,16,
    
        // BOTTOM
        20,21,22, 22,23,20
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
    // Creates a pyramid with a square base and triangular sides
    // Each side face needs its own normal for proper lighting
    
    std::vector<Vertex> vertices =
    {
        // BASE SQUARE (on XZ plane at Y=0)
        {{-0.5f, 0.0f, -0.5f}, {0,-1,0}},  // Vertex 0: Back-left
        {{ 0.5f, 0.0f, -0.5f}, {0,-1,0}},  // Vertex 1: Back-right
        {{ 0.5f, 0.0f,  0.5f}, {0,-1,0}},  // Vertex 2: Front-right
        {{-0.5f, 0.0f,  0.5f}, {0,-1,0}},  // Vertex 3: Front-left

        // APEX FOR BACK SIDE (with back-facing normal)
        {{0.0f, 1.0f, 0.0f}, {0.0f, 0.4472f, -0.8944f}},  // Vertex 4
        
        // APEX FOR RIGHT SIDE (with right-facing normal)
        {{0.0f, 1.0f, 0.0f}, {0.8944f, 0.4472f, 0.0f}},   // Vertex 5
        
        // APEX FOR FRONT SIDE (with front-facing normal)
        {{0.0f, 1.0f, 0.0f}, {0.0f, 0.4472f, 0.8944f}},   // Vertex 6
        
        // APEX FOR LEFT SIDE (with left-facing normal)
        {{0.0f, 1.0f, 0.0f}, {-0.8944f, 0.4472f, 0.0f}},  // Vertex 7
    };

    // Indices with proper winding order
    std::vector<uint32_t> indices =
    {
        // BASE (2 triangles forming square bottom)
        0, 2, 1,   // First triangle
        2, 0, 3,   // Second triangle
        
        // SIDE FACES (each triangle uses the apex with appropriate normal)
        0, 1, 4,   // Back side:  uses apex vertex 4
        1, 2, 5,   // Right side: uses apex vertex 5
        2, 3, 6,   // Front side: uses apex vertex 6
        3, 0, 7    // Left side:  uses apex vertex 7
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
    // Creates a flat circular disc using a triangle fan
    // segments: Number of edge segments (minimum 3)
    
    if (segments < 3) segments = 3;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    // CENTER VERTEX
    vertices.push_back({ {0, 0, 0}, {0, 1, 0} });

    float radius = 0.5f;
    float step = 6.28318530718f / segments;  // 2π / segments

    // Generate perimeter vertices
    for (uint32_t i = 0; i <= segments; ++i)
    {
        float t = step * i;
        float x = std::cos(t) * radius;
        float z = std::sin(t) * radius;
        vertices.push_back({ {x, 0, z}, {0, 1, 0} });
    }

    // Generate triangle fan indices
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

//
// ---------- PLANE (1x1 Quad) ----------
//
std::shared_ptr<Mesh> Mesh::CreatePlane()
{
    std::vector<Vertex> vertices =
    {
        {{-0.5f, 0.0f,  0.5f}, {0, 1, 0}},
        {{ 0.5f, 0.0f,  0.5f}, {0, 1, 0}},
        {{ 0.5f, 0.0f, -0.5f}, {0, 1, 0}},
        {{-0.5f, 0.0f, -0.5f}, {0, 1, 0}}
    };

    std::vector<uint32_t> indices = { 0, 1, 2, 2, 3, 0 };

    auto mesh = std::shared_ptr<Mesh>(new Mesh(vertices, indices));
    mesh->m_Type = PrimitiveType::Plane;
    return mesh;
}