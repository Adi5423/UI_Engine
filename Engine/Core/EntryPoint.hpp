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
#include <iostream>

// Forward declaration - must be implemented by the client application
extern Application* CreateApplication(ApplicationCommandLineArgs args);

int main(int argc, char** argv)
{
    // ========================================================================
    // PHASE 1: Pre-Initialization
    // ========================================================================
    std::cout << "===============================================\n";
    std::cout << "   Groove Engine - Initializing...\n";
    std::cout << "===============================================\n";

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
        std::cerr << "[FATAL] CreateApplication() returned nullptr!\n";
        return -1;
    }

    std::cout << "[Engine] Application created successfully.\n";

    // ========================================================================
    // PHASE 3: Run the Application
    // ========================================================================
    std::cout << "[Engine] Entering main loop...\n";
    std::cout << "===============================================\n\n";

    app->Run();

    // ========================================================================
    // PHASE 4: Shutdown
    // ========================================================================
    std::cout << "\n===============================================\n";
    std::cout << "[Engine] Shutting down...\n";
    
    delete app;
    
    std::cout << "[Engine] Application terminated successfully.\n";
    std::cout << "===============================================\n";

    return 0;
}
