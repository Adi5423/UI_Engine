#include "ImGuiLayer.hpp"

// ImGui core
#include <imgui.h>

// ImGui backends
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <iostream>

ImGuiLayer::ImGuiLayer() {}
ImGuiLayer::~ImGuiLayer() {}

void ImGuiLayer::OnAttach(GLFWwindow* window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleFonts;
    io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports;


    ImGui::GetMainViewport()->PlatformHandleRaw = window;
    ImGui::GetMainViewport()->PlatformHandle = window;

    // Fix window stacking issue (floating panels going behind main window)
    ImGui::GetIO().ConfigViewportsNoDecoration = false;
    ImGui::GetIO().ConfigViewportsNoTaskBarIcon = true;

    // Always draw platform windows above main window
    ImGuiStyle& styleFix = ImGui::GetStyle();
    styleFix.DisplayWindowPadding = ImVec2(0, 0);


    // Dark Theme
    // ----- Style: Hazel + Minimal Flat mix -----
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        //style.Colors[ImGuiCol_WindowBg].w = 1.0f;     //it breaks window Z-order (Creates separate OS windows without proper flags -> going behind main window)
    }

    style.WindowRounding = 4.0f;
    style.FrameRounding = 2.0f;
    style.TabRounding = 3.0f;

    style.ScrollbarRounding = 3.0f;
    style.GrabRounding = 2.0f;

    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;

    // Spacing
    style.WindowPadding = ImVec2(8, 8);
    style.FramePadding = ImVec2(6, 4);
    style.ItemSpacing = ImVec2(8, 6);

    // Colors (tuned for engine/editor vibe)
    ImVec4 bg = ImVec4(0.11f, 0.11f, 0.12f, 1.0f);
    ImVec4 bgDark = ImVec4(0.07f, 0.07f, 0.08f, 1.0f);
    ImVec4 bgLight = ImVec4(0.16f, 0.16f, 0.18f, 1.0f);
    ImVec4 accent = ImVec4(0.26f, 0.59f, 0.98f, 1.00f); // hazel/blue style
    ImVec4 accentSoft = ImVec4(0.20f, 0.45f, 0.80f, 1.00f);

    style.Colors[ImGuiCol_WindowBg] = bg;
    style.Colors[ImGuiCol_ChildBg] = bgDark;
    style.Colors[ImGuiCol_PopupBg] = bgDark;

    style.Colors[ImGuiCol_Border] = ImVec4(0, 0, 0, 0.5f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 0.0f);

    style.Colors[ImGuiCol_FrameBg] = bgLight;
    style.Colors[ImGuiCol_FrameBgHovered] = accentSoft;
    style.Colors[ImGuiCol_FrameBgActive] = accent;

    style.Colors[ImGuiCol_TitleBg] = bgDark;
    style.Colors[ImGuiCol_TitleBgActive] = bg;
    style.Colors[ImGuiCol_TitleBgCollapsed] = bgDark;

    style.Colors[ImGuiCol_MenuBarBg] = bgDark;

    style.Colors[ImGuiCol_Tab] = bgDark;
    style.Colors[ImGuiCol_TabHovered] = accentSoft;
    style.Colors[ImGuiCol_TabActive] = bg;
    style.Colors[ImGuiCol_TabUnfocused] = bgDark;
    style.Colors[ImGuiCol_TabUnfocusedActive] = bg;

    style.Colors[ImGuiCol_Header] = bgLight;
    style.Colors[ImGuiCol_HeaderHovered] = accentSoft;
    style.Colors[ImGuiCol_HeaderActive] = accent;

    style.Colors[ImGuiCol_Button] = bgLight;
    style.Colors[ImGuiCol_ButtonHovered] = accentSoft;
    style.Colors[ImGuiCol_ButtonActive] = accent;

    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0, 0, 0, 0);
    style.Colors[ImGuiCol_ResizeGripHovered] = accentSoft;
    style.Colors[ImGuiCol_ResizeGripActive] = accent;

    style.Colors[ImGuiCol_Separator] = ImVec4(0.25f, 0.25f, 0.28f, 1.0f);
    style.Colors[ImGuiCol_SeparatorHovered] = accentSoft;
    style.Colors[ImGuiCol_SeparatorActive] = accent;

    style.Colors[ImGuiCol_ScrollbarBg] = bg;
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.30f, 0.30f, 0.33f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = accentSoft;
    style.Colors[ImGuiCol_ScrollbarGrabActive] = accent;

    style.Colors[ImGuiCol_CheckMark] = accent;
    style.Colors[ImGuiCol_SliderGrab] = accentSoft;
    style.Colors[ImGuiCol_SliderGrabActive] = accent;

    // Viewport windows background
    style.Colors[ImGuiCol_DockingEmptyBg] = bgDark;
    style.Colors[ImGuiCol_DockingPreview] = accentSoft;


    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 450");

    m_Enabled = true;

    std::cout << "ImGui initialized.\n";
}

void ImGuiLayer::OnDetach()
{
    if (!m_Enabled)
        return;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiLayer::Begin()
{
    if (!m_Enabled)
        return;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiLayer::End()
{
    if (!m_Enabled)
        return;

    ImGuiIO& io = ImGui::GetIO();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Viewport windows (optional but looks professional)
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow* backup = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup);
    }
}
