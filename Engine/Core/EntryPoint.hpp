#pragma once

/**
 * ============================================================================
 * ENTRY POINT - The Heart of the Engine's Bootstrap System
 * ============================================================================
 * 
 * This is the universal entry point for all applications built on this engine.
 * Similar to how professional engines (Unreal, Unity, Hazel) work:
 * 
 * 1. The ENGINE owns `main()` - not the application
 * 2. The application defines `CreateApplication()` which returns their custom app
 * 3. The engine handles all initialization, running, and cleanup
 * 
 * This pattern gives the engine complete control over the application lifecycle
 * while allowing clients to customize behavior through inheritance.
 * 
 * Usage (in a client application):
 * --------------------------------
 * // MyApp.cpp
 * #include <Core/EntryPoint.hpp>
 * 
 * class MyGame : public Application { ... };
 * 
 * Application* CreateApplication(ApplicationCommandLineArgs args)
 * {
 *     return new MyGame();
 * }
 * 
 * That's it! The engine handles everything else.
 * ============================================================================
 */

#include "Application.hpp"
#include "Log.hpp"
#include <iostream>

// Forward declaration - must be implemented by the client application
extern Application* CreateApplication(ApplicationCommandLineArgs args);

int main(int argc, char** argv)
{
    // ========================================================================
    // PHASE 1: Pre-Initialization
    // ========================================================================
    Core::Log::Init();
    
    CORE_INFO("===============================================");
    CORE_INFO("   Groove Engine - Initializing...");
    CORE_INFO("===============================================");

    // ========================================================================
    // PHASE 2: Create Application
    // ========================================================================
    // Call into client code to create their application instance
    ApplicationCommandLineArgs args;
    args.Count = argc;
    args.Args = argv;

    Application* app = CreateApplication(args);
    if (!app)
    {
        CORE_FATAL("CreateApplication() returned nullptr!");
        return -1;
    }

    CORE_INFO("Application created successfully.");

    // ========================================================================
    // PHASE 3: Run the Application
    // ========================================================================
    CORE_INFO("Entering main loop...");
    CORE_INFO("===============================================\n");

    app->Run();

    // ========================================================================
    // PHASE 4: Shutdown
    // ========================================================================
    CORE_INFO("\n===============================================");
    CORE_INFO("Shutting down...");
    
    delete app;
    
    CORE_INFO("Application terminated successfully.");
    CORE_INFO("===============================================");

    return 0;
}
