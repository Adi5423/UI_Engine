set(EDITOR_SRC
    main.cpp
    Core/ImGuiLayer.cpp
    Core/EditorLayer.cpp
)

add_executable(UICheckEditor ${EDITOR_SRC})

target_include_directories(UICheckEditor PRIVATE
    ${CMAKE_SOURCE_DIR}/Engine
    ${CMAKE_SOURCE_DIR}/Editor
    ../vendor
)

target_link_libraries(UICheckEditor PRIVATE
    UICheckEngine
    imgui
)
