#include "Dockspace.h"
#include "Editor.h"
#include "commands/Commands.h"
#include "Constants.h"
#include "CommandLineOptions.h"
#include "window.h"
//
#include "widgets/PropertyEditor.h"
#include "widgets/StageOutliner.h"
#include "widgets/Timeline.h"
#include "widgets/LayerEditor.h"
#include "widgets/LayerHierarchyEditor.h"
#include "widgets/LayerEditor.h"
#include "widgets/ContentBrowser.h"
#include "widgets/PrimSpecEditor.h"
#include "widgets/LayerEditor.h"
#include "widgets/TextEditor.h"
#include "widgets/ModalDialogs.h"
#include "widgets/FileBrowser.h"
//
#include <pxr/base/arch/fileSystem.h>
#include <pxr/usd/sdf/fileFormat.h>
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

/// Modal dialog used to create a new layer
struct CreateUsdFileModalDialog : public ModalDialog {

    CreateUsdFileModalDialog(Editor &editor) : editor(editor), createStage(true) { ResetFileBrowserFilePath(); };

    void Draw() override {
        DrawFileBrowser();
        auto filePath = GetFileBrowserFilePath();
        ImGui::Checkbox("Open as stage", &createStage);
        if (FilePathExists()) {
            // ... could add other messages like permission denied, or incorrect extension
            ImGui::TextColored(ImVec4(1.0f, 0.1f, 0.1f, 1.0f), "Warning: overwriting");
        } else {
            if (!filePath.empty()) {
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "New stage: ");
            }
        }

        ImGui::Text("%s", filePath.c_str());
        DrawOkCancelModal([&]() {
            if (!filePath.empty()) {
                if (createStage) {
                    editor.CreateStage(filePath);
                } else {
                    editor.CreateLayer(filePath);
                }
            }
        });
    }

    const char *DialogId() const override { return "Create usd file"; }
    Editor &editor;
    bool createStage = true;
};

// Get usd known file format extensions and returns then prefixed with a dot and in a vector
static const std::vector<std::string> GetUsdValidExtensions() {
    const auto usdExtensions = SdfFileFormat::FindAllFileFormatExtensions();
    std::vector<std::string> validExtensions;
    auto addDot = [](const std::string &str) { return "." + str; };
    std::transform(usdExtensions.cbegin(), usdExtensions.cend(), std::back_inserter(validExtensions), addDot);
    return validExtensions;
}

/// Modal dialog to open a layer
struct OpenUsdFileModalDialog : public ModalDialog {

    OpenUsdFileModalDialog(Editor &editor) : editor(editor) { SetValidExtensions(GetUsdValidExtensions()); };
    ~OpenUsdFileModalDialog() override {}
    void Draw() override {
        DrawFileBrowser();

        if (FilePathExists()) {
            ImGui::Checkbox("Open as stage", &openAsStage);
            if (openAsStage) {
                ImGui::SameLine();
                ImGui::Checkbox("Load payloads", &openLoaded);
            }
        } else {
            ImGui::Text("Not found: ");
        }
        auto filePath = GetFileBrowserFilePath();
        ImGui::Text("%s", filePath.c_str());
        DrawOkCancelModal([&]() {
            if (!filePath.empty() && FilePathExists()) {
                if (openAsStage) {
                    editor.ImportStage(filePath, openLoaded);
                } else {
                    editor.ImportLayer(filePath);
                }
            }
        });
    }

    const char *DialogId() const override { return "Open layer"; }
    Editor &editor;
    bool openAsStage = true;
    bool openLoaded = true;
};

struct SaveLayerAs : public ModalDialog {

    SaveLayerAs(Editor &editor) : editor(editor){};
    ~SaveLayerAs() override {}
    void Draw() override {
        DrawFileBrowser();
        auto filePath = GetFileBrowserFilePath();
        if (FilePathExists()) {
            ImGui::TextColored(ImVec4(1.0f, 0.1f, 0.1f, 1.0f), "Overwrite: ");
        } else {
            ImGui::Text("Save to: ");
        }
        ImGui::Text("%s", filePath.c_str());
        DrawOkCancelModal([&]() { // On Ok ->
            if (!filePath.empty() && !FilePathExists()) {
                editor.SaveCurrentLayerAs(filePath);
            }
        });
    }

    const char *DialogId() const override { return "Save layer as"; }
    Editor &editor;
};

// setup docks
void Setup(Dockspace *dockspace, Editor *self, Viewport *viewport) {
    auto &_docks = dockspace->Docks();

    _docks.push_back(Dock("Stage viewport", &dockspace->Settings()._showViewport, [self, viewport](bool *p_open) {
        if (ImGui::Begin("Viewport", p_open)) {
            ImVec2 wsize = ImGui::GetWindowSize();
            viewport->SetSize(wsize.x, wsize.y - ViewportBorderSize); // for the next render
            viewport->Draw();
        }
        ImGui::End();
    }));

    _docks.push_back(Dock("Debug window", &dockspace->Settings()._showDebugWindow, [](bool *p_open) {
        if (ImGui::Begin("Debug window", p_open)) {
            // DrawDebugInfo();
            ImGui::Text("\xee\x81\x99"
                        " %.3f ms/frame  (%.1f FPS)",
                        1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        }
        ImGui::End();
    }));

    _docks.push_back(Dock("Stage property editor", &dockspace->Settings()._showPropertyEditor, [self, viewport](bool *p_open) {
        ImGuiWindowFlags windowFlags = 0 | ImGuiWindowFlags_MenuBar;
        if (ImGui::Begin("Stage property editor", p_open, windowFlags)) {
            if (auto stg = self->GetCurrentStage()) {
                auto prim = stg->GetPrimAtPath(GetSelectedPath(viewport->GetSelection()));
                DrawUsdPrimProperties(prim, viewport->GetCurrentTimeCode());
            }
        }
        ImGui::End();
    }));

    _docks.push_back(Dock("Stage outliner", &dockspace->Settings()._showOutliner, [self, viewport](bool *p_open) {
        ImGui::Begin("Stage outliner", p_open);
        DrawStageOutliner(self->GetCurrentStage(), viewport->GetSelection());
        ImGui::End();
    }));

    _docks.push_back(Dock("Stage timeline", &dockspace->Settings()._showTimeline, [self, viewport](bool *p_open) {
        ImGui::Begin("Timeline", p_open);
        UsdTimeCode tc = viewport->GetCurrentTimeCode();
        DrawTimeline(self->GetCurrentStage(), tc);
        viewport->SetCurrentTimeCode(tc);
        ImGui::End();
    }));

    _docks.push_back(Dock("Layer navigator", &dockspace->Settings()._showLayerEditor, [self](bool *p_open) {
        const auto &rootLayer = self->GetCurrentLayer();
        const std::string title("Layer navigation" +
                                (rootLayer ? (" - " + rootLayer->GetDisplayName() + (rootLayer->IsDirty() ? " *" : " ")) : "") +
                                "###Layer navigation");
        ImGui::Begin(title.c_str(), p_open);
        DrawLayerNavigation(rootLayer, self->GetSelectedPrimSpec());
        ImGui::End();
    }));

    _docks.push_back(Dock("Layer hierarchy", &dockspace->Settings()._showLayerHierarchyEditor, [self](bool *p_open) {
        const auto &rootLayer = self->GetCurrentLayer();
        const std::string title("Layer hierarchy " +
                                (rootLayer ? (" - " + rootLayer->GetDisplayName() + (rootLayer->IsDirty() ? " *" : " ")) : "") +
                                "###Layer hierarchy");
        ImGui::Begin(title.c_str(), p_open);
        DrawLayerPrimHierarchy(rootLayer, self->GetSelectedPrimSpec());
        ImGui::End();
    }));

    _docks.push_back(Dock("Layer stack", &dockspace->Settings()._showLayerStackEditor, [self](bool *p_open) {
        const auto &rootLayer = self->GetCurrentLayer();
        const std::string title("Layer stack " +
                                (rootLayer ? (" - " + rootLayer->GetDisplayName() + (rootLayer->IsDirty() ? " *" : " ")) : "") +
                                "###Layer stack");
        ImGui::Begin(title.c_str(), p_open);
        DrawLayerSublayers(rootLayer);
        ImGui::End();
    }));

    _docks.push_back(Dock("Content browser", &dockspace->Settings()._showContentBrowser, [self](bool *p_open) {
        ImGuiWindowFlags windowFlags = 0 | ImGuiWindowFlags_MenuBar;
        ImGui::Begin("Content browser", p_open, windowFlags);
        DrawContentBrowser(*self);
        ImGui::End();
    }));

    _docks.push_back(Dock("Layer property editor", &dockspace->Settings()._showPrimSpecEditor, [self](bool *p_open) {
        ImGui::Begin("Layer property editor", p_open);
        if (self->GetSelectedPrimSpec()) {
            DrawPrimSpecEditor(self->GetSelectedPrimSpec());
        } else {
            DrawLayerHeader(self->GetCurrentLayer());
        }
        ImGui::End();
    }));

    _docks.push_back(Dock("Layer text editor", &dockspace->Settings()._textEditor, [self](bool *p_open) {
        ImGui::Begin("Layer text editor", p_open);
        DrawTextEditor(self->GetCurrentLayer());
        ImGui::End();
    }));

    auto file = dockspace->GetMenu("File");
    auto has_layer = [self]() { return self->GetCurrentLayer() != SdfLayerRefPtr(); };
    file->items.push_front(
        {ICON_FA_SAVE " Save layer as", "CTRL+F", [self]() { DrawModalDialog<SaveLayerAs>(*self); }, has_layer});
    file->items.push_front({ICON_FA_SAVE " Save layer", "CTRL+S", [self]() { self->GetCurrentLayer()->Save(true); }, has_layer});
    file->items.push_front({});
    file->items.push_front({ICON_FA_FOLDER_OPEN " Open", "", [self]() { DrawModalDialog<OpenUsdFileModalDialog>(*self); }});
    file->items.push_front({ICON_FA_FILE " New", "", [self]() { DrawModalDialog<CreateUsdFileModalDialog>(*self); }});

    auto edit = dockspace->GetMenu("Edit");
    edit->items.push_front({"Redo", "CTRL+R", []() { ExecuteAfterDraw<RedoCommand>(); }});
    edit->items.push_front({"Undo", "CTRL+Z", []() { ExecuteAfterDraw<UndoCommand>(); }});
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
        Viewport viewport;
        editor.OnStageChanged = [&viewport](auto stage)
        {
            viewport.SetCurrentStage(stage);
        };

        Setup(&dockspace, &editor, &viewport);

        window->setOnDrop([&editor](int count, const char **paths) {
            for (int i = 0; i < count; ++i) {
                // make a drop event ?
                if (ArchGetFileLength(paths[i]) == 0) {
                    // if the file is empty, this is considered a new file
                    editor.CreateStage(std::string(paths[i]));
                } else {
                    editor.ImportLayer(std::string(paths[i]));
                }
            }
        });

        // Process command line options
        for (auto &stage : options.stages()) {
            editor.ImportStage(stage);
        }

        // Loop until the user closes the window
        int width;
        int height;
        while (window->newFrame(&width, &height)) {

            // render to texture
            viewport.Update();
            viewport.Render();

            // editor.HydraRender();

            // Render GUI next
            glViewport(0, 0, width, height);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            if (!dockspace.Render()) {
                break;
            }
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
