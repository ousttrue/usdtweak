#include "Dockspace.h"
#include "Editor.h"
#include "Commands.h"
#include "Constants.h"
#include "ResourcesLoader.h"
#include "CommandLineOptions.h"
#include "window.h"
//
#include "PropertyEditor.h"
#include "StageOutliner.h"
#include "Timeline.h"
#include "LayerEditor.h"
#include "LayerHierarchyEditor.h"
#include "LayerEditor.h"
#include "ContentBrowser.h"
#include "PrimSpecEditor.h"
#include "LayerEditor.h"
#include "TextEditor.h"
//
#include <imgui.h>
#include <iostream>

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

// setup docks
void Setup(std::list<Dock> &_docks, Editor *self) {
    _docks.push_back(Dock("Stage viewport", &self->Settings()._showViewport, [self](bool *p_open) {
        if (ImGui::Begin("Viewport", p_open)) {
            ImVec2 wsize = ImGui::GetWindowSize();
            self->GetViewport()->SetSize(wsize.x, wsize.y - ViewportBorderSize); // for the next render
            self->GetViewport()->Draw();
        }
        ImGui::End();
    }));

    _docks.push_back(Dock("Debug window", &self->Settings()._showDebugWindow, [](bool *p_open) {
        if (ImGui::Begin("Debug window", p_open)) {
            // DrawDebugInfo();
            ImGui::Text("\xee\x81\x99"
                        " %.3f ms/frame  (%.1f FPS)",
                        1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        }
        ImGui::End();
    }));

    _docks.push_back(Dock("Stage property editor", &self->Settings()._showPropertyEditor, [self](bool *p_open) {
        ImGuiWindowFlags windowFlags = 0 | ImGuiWindowFlags_MenuBar;
        if (ImGui::Begin("Stage property editor", p_open, windowFlags)) {
            if (auto stg = self->GetCurrentStage()) {
                auto prim = stg->GetPrimAtPath(GetSelectedPath(self->GetSelection()));
                DrawUsdPrimProperties(prim, self->GetViewport()->GetCurrentTimeCode());
            }
        }
        ImGui::End();
    }));

    _docks.push_back(Dock("Stage outliner", &self->Settings()._showOutliner, [self](bool *p_open) {
        ImGui::Begin("Stage outliner", p_open);
        DrawStageOutliner(self->GetCurrentStage(), self->GetSelection());
        ImGui::End();
    }));

    _docks.push_back(Dock("Stage timeline", &self->Settings()._showTimeline, [self](bool *p_open) {
        ImGui::Begin("Timeline", p_open);
        UsdTimeCode tc = self->GetViewport()->GetCurrentTimeCode();
        DrawTimeline(self->GetCurrentStage(), tc);
        self->GetViewport()->SetCurrentTimeCode(tc);
        ImGui::End();
    }));

    _docks.push_back(Dock("Layer navigator", &self->Settings()._showLayerEditor, [self](bool *p_open) {
        const auto &rootLayer = self->GetCurrentLayer();
        const std::string title("Layer navigation" +
                                (rootLayer ? (" - " + rootLayer->GetDisplayName() + (rootLayer->IsDirty() ? " *" : " ")) : "") +
                                "###Layer navigation");
        ImGui::Begin(title.c_str(), p_open);
        DrawLayerNavigation(rootLayer, self->GetSelectedPrimSpec());
        ImGui::End();
    }));

    _docks.push_back(Dock("Layer hierarchy", &self->Settings()._showLayerHierarchyEditor, [self](bool *p_open) {
        const auto &rootLayer = self->GetCurrentLayer();
        const std::string title("Layer hierarchy " +
                                (rootLayer ? (" - " + rootLayer->GetDisplayName() + (rootLayer->IsDirty() ? " *" : " ")) : "") +
                                "###Layer hierarchy");
        ImGui::Begin(title.c_str(), p_open);
        DrawLayerPrimHierarchy(rootLayer, self->GetSelectedPrimSpec());
        ImGui::End();
    }));

    _docks.push_back(Dock("Layer stack", &self->Settings()._showLayerStackEditor, [self](bool *p_open) {
        const auto &rootLayer = self->GetCurrentLayer();
        const std::string title("Layer stack " +
                                (rootLayer ? (" - " + rootLayer->GetDisplayName() + (rootLayer->IsDirty() ? " *" : " ")) : "") +
                                "###Layer stack");
        ImGui::Begin(title.c_str(), p_open);
        DrawLayerSublayers(rootLayer);
        ImGui::End();
    }));

    _docks.push_back(Dock("Content browser", &self->Settings()._showContentBrowser, [self](bool *p_open) {
        ImGuiWindowFlags windowFlags = 0 | ImGuiWindowFlags_MenuBar;
        ImGui::Begin("Content browser", p_open, windowFlags);
        DrawContentBrowser(*self);
        ImGui::End();
    }));

    _docks.push_back(Dock("Layer property editor", &self->Settings()._showPrimSpecEditor, [self](bool *p_open) {
        ImGui::Begin("Layer property editor", p_open);
        if (self->GetSelectedPrimSpec()) {
            DrawPrimSpecEditor(self->GetSelectedPrimSpec());
        } else {
            DrawLayerHeader(self->GetCurrentLayer());
        }
        ImGui::End();
    }));

    _docks.push_back(Dock("Layer text editor", &self->Settings()._textEditor, [self](bool *p_open) {
        ImGui::Begin("Layer text editor", p_open);
        DrawTextEditor(self->GetCurrentLayer());
        ImGui::End();
    }));
}

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

    Dockspace dockspace(window->get());

    { // Scope as the editor should be deleted before imgui and glfw, to release correctly the memory
        // Resource will load the font/textures/settings
        Editor editor;
        ResourcesLoader resources; // Assuming resources will be destroyed after editor

        Setup(dockspace.Docks(), &editor);

        window->setOnDrop(&editor, Editor::DropCallback);

        // Process command line options
        for (auto &stage : options.stages()) {
            editor.ImportStage(stage);
        }

        // Loop until the user closes the window
        int width;
        int height;
        while (window->newFrame(&width, &height)) {

            if (!editor.HydraRender()) {
                break;
            }

            // Render GUI next
            glViewport(0, 0, width, height);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            dockspace.Render();
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
