/**
 * ============================================================================
 * EDITOR ENTRY POINT
 * ============================================================================
 * 
 * This is the Editor's entry point file. Notice how clean this is!
 * 
 * The engine provides:
 *   - EntryPoint.hpp: Contains main() that calls CreateApplication()
 *   - Application: Base class with lifecycle methods
 * 
 * The Editor provides:
 *   - EditorApplication: Custom app class that overrides OnInit/OnUpdate/OnShutdown
 *   - CreateApplication(): Factory function that creates and returns our app
 * 
 * This is the same pattern used by professional engines like:
 *   - Hazel Engine
 *   - Unreal Engine (GameInstance)
 *   - Unity (MonoBehaviour lifecycle)
 * 
 * ============================================================================
 */

#include "Core/EditorApplication.hpp"

// Include the engine's entry point LAST - it defines main()
#include <Core/EntryPoint.hpp>

/**
 * CreateApplication
 * -----------------
 * This is the factory function that the engine calls from main().
 * It's the "contract" between the engine and your application.
 * 
 * The engine says: "Give me an Application*, and I'll run it for you."
 * 
 * @param args Command line arguments passed to the application
 * @return Pointer to the created application (ownership transfers to engine)
 */
Application* CreateApplication(ApplicationCommandLineArgs args)
{
    ApplicationSpecification spec;
    spec.Name = "Groove Engine Editor";
    spec.WindowWidth = 1280;
    spec.WindowHeight = 720;
    spec.CommandLineArgs = args;

    return new EditorApplication(spec);
}

