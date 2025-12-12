#pragma once

/**
 * ============================================================================
 * APPLICATION - The Core Engine Runtime
 * ============================================================================
 * 
 * This is the base class for all applications built on the engine.
 * Professional game engines use this pattern:
 * 
 * - Engine provides Application base class
 * - Clients (Editor, Game) inherit and override virtual methods
 * - Engine controls the main loop, clients just respond to lifecycle events
 * 
 * Lifecycle:
 * ----------
 * 1. Construction (via CreateApplication)
 * 2. OnInit() - called once before the main loop
 * 3. OnUpdate(deltaTime) - called every frame
 * 4. OnShutdown() - called once after the loop ends
 * 5. Destruction
 * 
 * ============================================================================
 */

#include "Window.hpp"
#include "Events/Event.hpp"
#include "Events/ApplicationEvent.hpp"
#include "LayerStack.hpp"
#include <string>
#include <memory>

// ============================================================================
// Command Line Arguments
// ============================================================================
struct ApplicationCommandLineArgs
{
    int Count = 0;
    char** Args = nullptr;

    const char* operator[](int index) const
    {
        if (index < 0 || index >= Count) return "";
        return Args[index];
    }
};

// ============================================================================
// Application Specification
// ============================================================================
struct ApplicationSpecification
{
    std::string Name = "Groove Engine Application";
    std::string WorkingDirectory = "";
    uint32_t WindowWidth = 1280;
    uint32_t WindowHeight = 720;
    ApplicationCommandLineArgs CommandLineArgs;
};

// ============================================================================
// Application Base Class
// ============================================================================
class Application
{
public:
    Application(const ApplicationSpecification& spec = ApplicationSpecification());
    virtual ~Application();

    // ========================================================================
    // Lifecycle Methods (Override in your application)
    // ========================================================================
    
    /**
     * Called once after the application is constructed but before 
     * the main loop starts. Use this to initialize your systems.
     */
    virtual void OnInit() {}
    
    /**
     * Called every frame during the main loop.
     * @param deltaTime Time elapsed since last frame (in seconds)
     */
    virtual void OnUpdate(float deltaTime) {}
    
    /**
     * Called once after the main loop ends, before destruction.
     * Use this to cleanup your systems.
     */
    virtual void OnShutdown() {}

    // ========================================================================
    // Engine API
    // ========================================================================
    
    /**
     * Runs the main application loop. Called by EntryPoint.
     */
    void Run();

    /**
     * The Main Event Handler.
     * Dispatches events to layers and handles internal engine events.
     */
    void OnEvent(EventSystem::Event& e);
    
    /**
     * Request the application to close. The loop will end gracefully.
     */
    void Close() { m_Running = false; }

    /**
     * Check if the application is still running.
     */
    bool IsRunning() const { return m_Running; }

    /**
     * Get the application window.
     */
    Window* GetWindow() const { return m_Window.get(); }

    /**
     * Get the application specification.
     */
    const ApplicationSpecification& GetSpecification() const { return m_Specification; }

    /**
     * Get the singleton application instance.
     */
    static Application& Get() { return *s_Instance; }

    LayerStack& GetLayerStack() { return m_LayerStack; }

    // Layer System wrappers
    void PushLayer(Layer* layer);
    void PushOverlay(Layer* layer);

private:
    bool OnWindowClose(EventSystem::WindowCloseEvent& e);
    bool OnWindowResize(EventSystem::WindowResizeEvent& e);

    ApplicationSpecification m_Specification;
    std::unique_ptr<Window> m_Window;
    bool m_Running = true;
    bool m_Minimized = false;
    float m_LastFrameTime = 0.0f;

    LayerStack m_LayerStack;

    static Application* s_Instance;
};
