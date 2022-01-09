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

PXR_NAMESPACE_USING_DIRECTIVE

int main(int argc, const char **argv) {

    CommandLineOptions options(argc, argv);

    // Initialize python
#ifdef WANTS_PYTHON
    Py_SetProgramName(argv[0]);
    Py_Initialize();
#endif

    // Initialize glfw
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#else
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
#endif

    /* Create a windowed mode window and its OpenGL context */
    int width = InitialWindowWidth;
    int height = InitialWindowHeight;
#ifdef DISABLE_DOUBLE_BUFFER
    glfwWindowHint(GLFW_DOUBLEBUFFER, GL_FALSE);
#endif
    GLFWwindow *window = glfwCreateWindow(width, height, "USD Tweak", NULL, NULL);
    if (!window) {
        std::cerr << "unable to create a window, exiting" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    // Init glew with USD
    GarchGLApiLoad();
    GlfContextCaps::InitInstance();
    std::cout << glGetString(GL_VENDOR) << std::endl;
    std::cout << glGetString(GL_RENDERER) << std::endl;
    std::cout << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    std::cout << "Hydra enabled : " << UsdImagingGLEngine::IsHydraEnabled() << std::endl;
    // GlfRegisterDefaultDebugOutputMessageCallback();

    // Create a context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = nullptr;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    //ImGuiStyle& style = ImGui::GetStyle();
    //style.Colors[ImGuiCol_Tab] = style.Colors[ImGuiCol_FrameBg];
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigWindowsMoveFromTitleBarOnly = true;



    { // Scope as the editor should be deleted before imgui and glfw, to release correctly the memory
        // Resource will load the font/textures/settings
        ResourcesLoader resources; // Assuming resources will be destroyed after editor
        Editor editor;

        glfwSetWindowUserPointer(window, &editor);
        glfwSetDropCallback(window, Editor::DropCallback);

        // Process command line options
        for (auto& stage : options.stages()) {
            editor.ImportStage(stage);
        }

        // Loop until the user closes the window
        while (!glfwWindowShouldClose(window) && !editor.ShutdownRequested()) {

            // Poll and process events
            glfwMakeContextCurrent(window);
            glfwPollEvents();

            // Render the viewports first as textures
            editor.HydraRender();

            // Render GUI next
            glfwGetFramebufferSize(window, &width, &height);
            glViewport(0, 0, width, height);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            editor.Draw();
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

#ifndef DISABLE_DOUBLE_BUFFER
            // Swap front and back buffers
            glfwSwapBuffers(window);
#else
            glFlush();
#endif
            // This forces to wait for the gpu commands to finish.
            // Normally not required but it fixes a pcoip driver issue
            glFinish();

            // Process edition commands
            ExecuteCommands();
        }

        glfwSetWindowUserPointer(window, nullptr);
    }

    // Shutdown imgui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Shutdown glfw
    glfwDestroyWindow(window);
    glfwTerminate();

#ifdef WANTS_PYTHON
    Py_Finalize();
#endif

    return 0;
}
