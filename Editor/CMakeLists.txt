set(EDITOR_SRC
    main.cpp
    Core/EditorApplication.cpp
    Core/EditorApplication.hpp
    Core/ImGuiLayer.cpp
    Core/EditorLayer.cpp
    Core/EditorBridge.cpp
    Core/ThemeSettings.cpp
    Core/WindowLayout.cpp
    Core/WindowLayout.hpp
    Core/ImGuiLayer.hpp
)

add_executable(UICheckEditor ${EDITOR_SRC})

target_include_directories(UICheckEditor PRIVATE
    ${CMAKE_SOURCE_DIR}/Engine
    ${CMAKE_SOURCE_DIR}/Editor
    ${CMAKE_SOURCE_DIR}/vendor
    ${CMAKE_SOURCE_DIR}/vendor/glm
    ${CMAKE_SOURCE_DIR}/vendor/stb
    ${CMAKE_SOURCE_DIR}/vendor/entt/single_include
    ${CMAKE_SOURCE_DIR}/vendor/json
)

target_link_libraries(UICheckEditor PRIVATE
    UICheckEngine
    imgui
    ImGuizmo
)
