// this will be deleted. (old structural code)

#include "Engine/Core/Application.hpp"

Application* CreateApplication()
{
    return new Application();
}

int main()
{
    auto app = CreateApplication();
    app->Run();
    delete app;
    return 0;
}
