#pragma once

#include <glm/glm.hpp>

class Renderer
{
public:
    static void Init();
    static void BeginScene();
    static void EndScene();

    static void Clear(const glm::vec4& color);
};
