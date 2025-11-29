#pragma once
#include <string>

class EditorLayer
{
public:
    EditorLayer();
    ~EditorLayer();

    void OnAttach();
    void OnDetach();
    void OnImGuiRender(); // Called every frame to draw panels
};
