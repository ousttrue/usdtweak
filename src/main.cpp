#include <iostream>
#include <pxr/base/plug/registry.h>
#include <pxr/imaging/glf/contextCaps.h>
#include <pxr/imaging/glf/simpleLight.h>
#include <pxr/imaging/glf/diagnostic.h>
#include "Editor.h"
#include "Viewport.h"
#include "Commands.h"
#include "Constants.h"
#include "ResourcesLoader.h"
#include "CommandLineOptions.h"
#include "Gui.h"
#include "window.h"

PXR_NAMESPACE_USING_DIRECTIVE

int main(int argc, const char **argv) {

    CommandLineOptions options(argc, argv);

    // Initialize python
#ifdef WANTS_PYTHON
    Py_SetProgramName(argv[0]);
    Py_Initialize();
#endif

    /* Create a windowed mode window and its OpenGL context */
    auto window = Window::create(InitialWindowWidth, InitialWindowHeight);
    if (!window) {
        return 1;
    }

    // Init glew with USD
    GarchGLApiLoad();
    GlfContextCaps::InitInstance();
    std::cout << glGetString(GL_VENDOR) << std::endl;
    std::cout << glGetString(GL_RENDERER) << std::endl;
    std::cout << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    std::cout << "Hydra enabled : " << UsdImagingGLEngine::IsHydraEnabled() << std::endl;
    // GlfRegisterDefaultDebugOutputMessageCallback();

    Gui gui(window->get());

    { // Scope as the editor should be deleted before imgui and glfw, to release correctly the memory
        // Resource will load the font/textures/settings
        ResourcesLoader resources; // Assuming resources will be destroyed after editor
        Editor editor;

        window->setOnDrop(&editor, Editor::DropCallback);

        // Process command line options
        for (auto &stage : options.stages()) {
            editor.ImportStage(stage);
        }

        // Loop until the user closes the window
        int width;
        int height;
        while (window->newFrame(&width, &height) && !editor.ShutdownRequested()) {

            // Render the viewports first as textures
            editor.HydraRender();

            // Render GUI next
            glViewport(0, 0, width, height);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            gui.beginFrame();
            editor.Draw();
            gui.endFrame();

            // This forces to wait for the gpu commands to finish.
            // Normally not required but it fixes a pcoip driver issue
            glFinish();
            window->flush();

            // Process edition commands
            ExecuteCommands();
        }
    }

#ifdef WANTS_PYTHON
    Py_Finalize();
#endif

    return 0;
}
