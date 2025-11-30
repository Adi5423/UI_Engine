#include "Renderer.hpp"
#include <glad/glad.h>

void Renderer::Init()
{
    glEnable(GL_DEPTH_TEST);
}

void Renderer::BeginScene()
{
}

void Renderer::EndScene()
{
}

void Renderer::Clear(const glm::vec4& color)
{
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
