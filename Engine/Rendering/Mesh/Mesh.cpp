#include "Mesh.hpp"
#include <vector>
#include <cmath>

//
// ---------- 3D CUBE ----------
//
std::shared_ptr<Mesh> Mesh::CreateCube()
{
    // Vertex format: { {X, Y, Z}, {NormalX, NormalY, NormalZ} }
    // 
    // Position values (first triplet):
    //   X: -0.5 to 0.5 → Controls cube width (left-right). Increase for wider, decrease for narrower
    //   Y: -0.5 to 0.5 → Controls cube height (bottom-top). Increase for taller, decrease for shorter
    //   Z: -0.5 to 0.5 → Controls cube depth (back-front). Increase for deeper, decrease for shallower
    //
    // Normal values (second triplet):
    //   Points outward from face for lighting calculations
    //   {0,0,1} = facing forward (+Z), {0,0,-1} = facing backward (-Z)
    //   {1,0,0} = facing right (+X),   {-1,0,0} = facing left (-X)
    //   {0,1,0} = facing up (+Y),      {0,-1,0} = facing down (-Y)
    //   Changing normals will affect how light reflects off the surface
    
    std::vector<Vertex> vertices =
    {
        // FRONT FACE (facing toward +Z direction)
        // Increasing Z value (0.5f → 1.0f) moves this face forward
        {{-0.5f,-0.5f, 0.5f}, {0,0,1}},  // Vertex 0: Bottom-left-front corner
        {{ 0.5f,-0.5f, 0.5f}, {0,0,1}},  // Vertex 1: Bottom-right-front corner
        {{ 0.5f, 0.5f, 0.5f}, {0,0,1}},  // Vertex 2: Top-right-front corner
        {{-0.5f, 0.5f, 0.5f}, {0,0,1}},  // Vertex 3: Top-left-front corner

        // BACK FACE (facing toward -Z direction)
        // Decreasing Z value (-0.5f → -1.0f) moves this face backward
        {{-0.5f,-0.5f,-0.5f}, {0,0,-1}}, // Vertex 4: Bottom-left-back corner
        {{ 0.5f,-0.5f,-0.5f}, {0,0,-1}}, // Vertex 5: Bottom-right-back corner
        {{ 0.5f, 0.5f,-0.5f}, {0,0,-1}}, // Vertex 6: Top-right-back corner
        {{-0.5f, 0.5f,-0.5f}, {0,0,-1}}, // Vertex 7: Top-left-back corner

        // LEFT FACE (facing toward -X direction)
        // Decreasing X value (-0.5f → -1.0f) moves this face left
        {{-0.5f,-0.5f,-0.5f}, {-1,0,0}}, // Vertex 8: Bottom-left-back corner (same position as 4)
        {{-0.5f,-0.5f, 0.5f}, {-1,0,0}}, // Vertex 9: Bottom-left-front corner (same position as 0)
        {{-0.5f, 0.5f, 0.5f}, {-1,0,0}}, // Vertex 10: Top-left-front corner (same position as 3)
        {{-0.5f, 0.5f,-0.5f}, {-1,0,0}}, // Vertex 11: Top-left-back corner (same position as 7)

        // RIGHT FACE (facing toward +X direction)
        // Increasing X value (0.5f → 1.0f) moves this face right
        {{ 0.5f,-0.5f,-0.5f}, {1,0,0}},  // Vertex 12: Bottom-right-back corner (same position as 5)
        {{ 0.5f,-0.5f, 0.5f}, {1,0,0}},  // Vertex 13: Bottom-right-front corner (same position as 1)
        {{ 0.5f, 0.5f, 0.5f}, {1,0,0}},  // Vertex 14: Top-right-front corner (same position as 2)
        {{ 0.5f, 0.5f,-0.5f}, {1,0,0}},  // Vertex 15: Top-right-back corner (same position as 6)

        // TOP FACE (facing toward +Y direction)
        // Increasing Y value (0.5f → 1.0f) moves this face upward
        {{-0.5f, 0.5f, 0.5f}, {0,1,0}},  // Vertex 16: Top-left-front corner (same position as 3)
        {{ 0.5f, 0.5f, 0.5f}, {0,1,0}},  // Vertex 17: Top-right-front corner (same position as 2)
        {{ 0.5f, 0.5f,-0.5f}, {0,1,0}},  // Vertex 18: Top-right-back corner (same position as 6)
        {{-0.5f, 0.5f,-0.5f}, {0,1,0}},  // Vertex 19: Top-left-back corner (same position as 7)

        // BOTTOM FACE (facing toward -Y direction)
        // Decreasing Y value (-0.5f → -1.0f) moves this face downward
        {{-0.5f,-0.5f, 0.5f}, {0,-1,0}}, // Vertex 20: Bottom-left-front corner (same position as 0)
        {{ 0.5f,-0.5f, 0.5f}, {0,-1,0}}, // Vertex 21: Bottom-right-front corner (same position as 1)
        {{ 0.5f,-0.5f,-0.5f}, {0,-1,0}}, // Vertex 22: Bottom-right-back corner (same position as 5)
        {{-0.5f,-0.5f,-0.5f}, {0,-1,0}}, // Vertex 23: Bottom-left-back corner (same position as 4)
    };

    // Indices define triangles by referencing vertex indices
    // Each group of 3 indices forms one triangle
    // Order matters: Counter-clockwise winding for front-facing (visible) triangles
    // Reversing order (e.g., 0,2,1 instead of 0,1,2) flips the triangle's visible side
    std::vector<uint32_t> indices =
    {
        // FRONT FACE (2 triangles)
        // Triangle 1: vertices 0→1→2 (bottom-left → bottom-right → top-right)
        // Triangle 2: vertices 2→3→0 (top-right → top-left → bottom-left)
         0, 1, 2,   2, 3, 0,
        
        // BACK FACE (2 triangles)
         4, 5, 6,   6, 7, 4,
        
        // LEFT FACE (2 triangles)
         8, 9,10,  10,11, 8,
        
        // RIGHT FACE (2 triangles)
        12,13,14,  14,15,12,
        
        // TOP FACE (2 triangles)
        16,17,18,  18,19,16,
        
        // BOTTOM FACE (2 triangles)
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
    // Creates a pyramid with a square base and triangular sides converging to a point
    // 
    // BASE VERTICES (Y = 0, on the ground plane):
    //   Changing the 0.0f (Y coordinate) moves the entire base up or down
    //   Changing ±0.5f (X and Z) changes the base size:
    //     Increase (e.g., 0.5f → 1.0f) = larger base
    //     Decrease (e.g., 0.5f → 0.25f) = smaller base
    //
    // APEX VERTEX (top point):
    //   First value (X = 0.0f): Center horizontally. Change to shift peak left/right
    //   Second value (Y = 1.0f): Height of pyramid. Increase for taller, decrease for shorter
    //   Third value (Z = 0.0f): Center depth-wise. Change to shift peak forward/back
    //
    // NORMALS:
    //   Base: {0,-1,0} points downward (ground facing)
    //   Apex: {0,1,0} points upward (used for side faces in this simple version)
    //   For accurate lighting on sides, each side should have its own calculated normal
    
    std::vector<Vertex> vertices =
    {
        // BASE SQUARE (4 corners on the XZ plane at Y=0)
        // These form a square base: changing ±0.5f values changes base width/depth
        {{-0.5f, 0.0f, -0.5f}, {0,-1,0}},  // Vertex 0: Back-left   corner (X-, Z-)
        {{ 0.5f, 0.0f, -0.5f}, {0,-1,0}},  // Vertex 1: Back-right  corner (X+, Z-)
        {{ 0.5f, 0.0f,  0.5f}, {0,-1,0}},  // Vertex 2: Front-right corner (X+, Z+)
        {{-0.5f, 0.0f,  0.5f}, {0,-1,0}},  // Vertex 3: Front-left  corner (X-, Z+)
        // Changing Y from 0.0f (e.g., to -0.2f) lowers base, or to 0.5f raises it off ground

        // APEX (tip of pyramid)
        // X=0.0f: Centered horizontally. Change to move peak left (negative) or right (positive)
        // Y=1.0f: Height above base. Increase for taller pyramid, decrease for flatter
        // Z=0.0f: Centered depth. Change to move peak back (negative) or forward (positive)
        {{0.0f, 1.0f, 0.0f}, {0,1,0}},     // Vertex 4: Apex (top point)
        // Example: {{0.2f, 1.5f, -0.1f}, {0,1,0}} makes a taller pyramid leaning right-backward
    };

    // Indices connect vertices into triangles
    // Each group of 3 numbers references vertex indices to form one triangle
    // Triangle winding order (counter-clockwise) determines which side is "front-facing"
    std::vector<uint32_t> indices =
    {
        // BASE (bottom of pyramid, 2 triangles forming a square)
        // Triangle 1: vertices 0→1→2 (back-left → back-right → front-right)
        // Triangle 2: vertices 2→3→0 (front-right → front-left → back-left)
        0, 1, 2,   // First triangle of base
        2, 3, 0,   // Second triangle of base
        // Together these form a solid square bottom

        // SIDE FACES (4 triangular sides, each connecting 2 base vertices to apex)
        // Each side is one triangle connecting a base edge to the apex (vertex 4)
        0, 1, 4,   // Back   side: back-left → back-right → apex
        1, 2, 4,   // Right  side: back-right → front-right → apex
        2, 3, 4,   // Front  side: front-right → front-left → apex
        3, 0, 4    // Left   side: front-left → back-left → apex
        // Reversing any triangle (e.g., 4,1,0 instead of 0,1,4) would flip it inside-out
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
    // 
    // PARAMETERS:
    //   segments: Number of edge segments (triangular wedges)
    //     - Higher value = smoother circle, more triangles, more GPU cost
    //     - Lower value = more angular (hexagon, octagon, etc.)
    //     - Minimum enforced: 3 (creates a triangle)
    
    if (segments < 3) segments = 3;  // Safeguard: minimum 3 segments for valid circle

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    // CENTER VERTEX (origin of the circle, hub of triangle fan)
    // First triplet {0,0,0}: Position at world origin
    //   Change X (first 0) to shift circle left/right: {-2,0,0} moves left 2 units
    //   Change Y (second 0) to raise/lower: {0,1,0} lifts circle 1 unit up
    //   Change Z (third 0) to move forward/back: {0,0,5} moves forward 5 units
    // Second triplet {0,1,0}: Normal pointing upward (+Y)
    //   {0,-1,0} would make it face downward (useful for ground underneath)
    //   {0,0,1} would orient it vertically facing forward (wall/billboard)
    vertices.push_back({ {0, 0, 0}, {0, 1, 0} });  // Vertex 0: Center

    // RADIUS: Controls size of circle
    // 0.5f = half-unit radius (1 unit diameter)
    // Increase (e.g., 2.0f) for larger circle, decrease (e.g., 0.1f) for smaller
    float radius = 0.5f;

    // STEP: Angle increment per segment (in radians)
    // 6.28318530718f ≈ 2π (full circle in radians)
    // Divided by segments = angle between each perimeter vertex
    // More segments = smaller step = smoother circle
    float step = 6.28318530718f / segments;

    // Generate perimeter vertices in a circle
    // Loop runs segments+1 times to close the circle (first and last vertices overlap)
    for (uint32_t i = 0; i <= segments; ++i)
    {
        // Calculate angle for this vertex
        // t increases by 'step' each iteration, sweeping full circle
        float t = step * i;

        // Convert polar coordinates (angle, radius) to Cartesian (x, z)
        // cos(t) * radius: X coordinate (horizontal position on circle)
        //   - At t=0°: x=radius (rightmost point)
        //   - At t=90°: x=0 (center)
        //   - At t=180°: x=-radius (leftmost point)
        // sin(t) * radius: Z coordinate (depth position on circle)
        //   - At t=0°: z=0 (center depth)
        //   - At t=90°: z=radius (frontmost point)
        //   - At t=270°: z=-radius (backmost point)
        float x = std::cos(t) * radius;  // Horizontal offset from center
        float z = std::sin(t) * radius;  // Depth offset from center

        // Add perimeter vertex
        // Position: {x, 0, z} - lies on XZ plane (Y=0) at 'radius' distance from center
        //   Changing the 0 (Y) to 0.5f would create a raised ring/cylinder cross-section
        // Normal: {0, 1, 0} - pointing upward for flat disc
        //   Change to {0, -1, 0} to flip orientation (face downward)
        //   Change to {x/radius, 0, z/radius} for outward-pointing normals (like a cylinder)
        vertices.push_back({ {x, 0, z}, {0, 1, 0} });
    }

    // Generate triangle indices in a fan pattern
    // Each triangle connects:
    //   - Center vertex (0)
    //   - Current perimeter vertex (i)
    //   - Next perimeter vertex (i+1)
    // This creates 'segments' triangular wedges radiating from center
    for (uint32_t i = 1; i <= segments; i++)
    {
        indices.push_back(0);       // Center (hub of fan)
        indices.push_back(i);       // Current edge vertex
        indices.push_back(i + 1);   // Next edge vertex (connects to current)
        
        // Winding order: 0 → i → i+1 (counter-clockwise from above)
        // Reversing (e.g., 0, i+1, i) would flip the circle upside-down (visible from below)
    }
    // Result: 'segments' triangles forming a complete disc

    auto mesh = std::shared_ptr<Mesh>(new Mesh(vertices, indices));
    mesh->m_Type = PrimitiveType::Circle;
    return mesh;
}
