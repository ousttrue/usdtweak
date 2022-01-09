#include <iostream>
#include "Editor.h"
#include "Commands.h"
#include "Constants.h"
#include "ResourcesLoader.h"
#include "CommandLineOptions.h"
#include "window.h"


#ifdef WANTS_PYTHON
class Python {
  public:
    Python(const char **argv) {
        Py_SetProgramName(argv[0]);
        Py_Initialize();
    }
    ~Python() { Py_Finalize(); }
};
#endif

int main(int argc, const char **argv) {

    CommandLineOptions options(argc, argv);

    // Initialize python
#ifdef WANTS_PYTHON
    Python python(argv);
#endif

    /* Create a windowed mode window and its OpenGL context */
    auto window = Window::create(InitialWindowWidth, InitialWindowHeight);
    if (!window) {
        return 1;
    }

    { // Scope as the editor should be deleted before imgui and glfw, to release correctly the memory
        // Resource will load the font/textures/settings
        Editor editor(window->get());
        ResourcesLoader resources; // Assuming resources will be destroyed after editor

        window->setOnDrop(&editor, Editor::DropCallback);

        // Process command line options
        for (auto &stage : options.stages()) {
            editor.ImportStage(stage);
        }

        // Loop until the user closes the window
        int width;
        int height;
        while (window->newFrame(&width, &height)) {

            if (!editor.Update()) {
                break;
            }

            // Render GUI next
            glViewport(0, 0, width, height);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            editor.Render();
            // This forces to wait for the gpu commands to finish.
            // Normally not required but it fixes a pcoip driver issue
            glFinish();
            window->flush();

            // Process edition commands
            ExecuteCommands();
        }
    }

    return 0;
}
