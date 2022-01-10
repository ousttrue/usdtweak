#include "Dockspace.h"
#include "stage/StageLoader.h"
#include "stage/commands/Commands.h"
#include "viewport/Viewport.h"
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

    CreateUsdFileModalDialog(StageLoader &editor) : editor(editor), createStage(true) { ResetFileBrowserFilePath(); };

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
    StageLoader &editor;
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

    OpenUsdFileModalDialog(StageLoader &editor) : editor(editor) { SetValidExtensions(GetUsdValidExtensions()); };
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
    StageLoader &editor;
    bool openAsStage = true;
    bool openLoaded = true;
};

struct SaveLayerAs : public ModalDialog {

    SaveLayerAs(StageLoader &editor) : editor(editor){};
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
    StageLoader &editor;
};

// setup docks
void Setup(Dockspace *dockspace, StageLoader *editor, HydraRenderer *viewport) {
    auto &_docks = dockspace->Docks();

    _docks.push_back(Dock("Stage viewport", &dockspace->Settings()._showViewport, [editor, viewport](bool *p_open) {
        if (ImGui::Begin("Viewport", p_open)) {
            ImVec2 wsize = ImGui::GetWindowSize();
            viewport->Draw(editor->GetCurrentStage(), editor->GetSelection(), static_cast<int>(wsize.x), static_cast<int>(wsize.y - ViewportBorderSize));
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

    _docks.push_back(Dock("Stage property editor", &dockspace->Settings()._showPropertyEditor, [editor, viewport](bool *p_open) {
        ImGuiWindowFlags windowFlags = 0 | ImGuiWindowFlags_MenuBar;
        if (ImGui::Begin("Stage property editor", p_open, windowFlags)) {
            if (auto stg = editor->GetCurrentStage()) {
                auto prim = stg->GetPrimAtPath(GetSelectedPath(editor->GetSelection()));
                DrawUsdPrimProperties(prim, viewport->GetCurrentTimeCode());
            }
        }
        ImGui::End();
    }));

    _docks.push_back(Dock("Stage outliner", &dockspace->Settings()._showOutliner, [editor, viewport](bool *p_open) {
        ImGui::Begin("Stage outliner", p_open);
        DrawStageOutliner(editor->GetCurrentStage(), editor->GetSelection());
        ImGui::End();
    }));

    _docks.push_back(Dock("Stage timeline", &dockspace->Settings()._showTimeline, [editor, viewport](bool *p_open) {
        ImGui::Begin("Timeline", p_open);
        UsdTimeCode tc = viewport->GetCurrentTimeCode();
        DrawTimeline(editor->GetCurrentStage(), tc);
        viewport->SetCurrentTimeCode(tc);
        ImGui::End();
    }));

    _docks.push_back(Dock("Layer navigator", &dockspace->Settings()._showLayerEditor, [editor](bool *p_open) {
        const auto &rootLayer = editor->GetCurrentLayer();
        const std::string title("Layer navigation" +
                                (rootLayer ? (" - " + rootLayer->GetDisplayName() + (rootLayer->IsDirty() ? " *" : " ")) : "") +
                                "###Layer navigation");
        ImGui::Begin(title.c_str(), p_open);
        DrawLayerNavigation(rootLayer, editor->GetSelectedPrimSpec());
        ImGui::End();
    }));

    _docks.push_back(Dock("Layer hierarchy", &dockspace->Settings()._showLayerHierarchyEditor, [editor](bool *p_open) {
        const auto &rootLayer = editor->GetCurrentLayer();
        const std::string title("Layer hierarchy " +
                                (rootLayer ? (" - " + rootLayer->GetDisplayName() + (rootLayer->IsDirty() ? " *" : " ")) : "") +
                                "###Layer hierarchy");
        ImGui::Begin(title.c_str(), p_open);
        DrawLayerPrimHierarchy(rootLayer, editor->GetSelectedPrimSpec());
        ImGui::End();
    }));

    _docks.push_back(Dock("Layer stack", &dockspace->Settings()._showLayerStackEditor, [editor](bool *p_open) {
        const auto &rootLayer = editor->GetCurrentLayer();
        const std::string title("Layer stack " +
                                (rootLayer ? (" - " + rootLayer->GetDisplayName() + (rootLayer->IsDirty() ? " *" : " ")) : "") +
                                "###Layer stack");
        ImGui::Begin(title.c_str(), p_open);
        DrawLayerSublayers(rootLayer);
        ImGui::End();
    }));

    _docks.push_back(Dock("Content browser", &dockspace->Settings()._showContentBrowser, [editor](bool *p_open) {
        ImGuiWindowFlags windowFlags = 0 | ImGuiWindowFlags_MenuBar;
        ImGui::Begin("Content browser", p_open, windowFlags);
        DrawContentBrowser(*editor);
        ImGui::End();
    }));

    _docks.push_back(Dock("Layer property editor", &dockspace->Settings()._showPrimSpecEditor, [editor](bool *p_open) {
        ImGui::Begin("Layer property editor", p_open);
        if (editor->GetSelectedPrimSpec()) {
            DrawPrimSpecEditor(editor->GetSelectedPrimSpec());
        } else {
            DrawLayerHeader(editor->GetCurrentLayer());
        }
        ImGui::End();
    }));

    _docks.push_back(Dock("Layer text editor", &dockspace->Settings()._textEditor, [editor](bool *p_open) {
        ImGui::Begin("Layer text editor", p_open);
        DrawTextEditor(editor->GetCurrentLayer());
        ImGui::End();
    }));

    auto file = dockspace->GetMenu("File");
    auto has_layer = [editor]() { return editor->GetCurrentLayer() != SdfLayerRefPtr(); };
    file->items.push_front(
        {ICON_FA_SAVE " Save layer as", "CTRL+F", [editor]() { DrawModalDialog<SaveLayerAs>(*editor); }, has_layer});
    file->items.push_front(
        {ICON_FA_SAVE " Save layer", "CTRL+S", [editor]() { editor->GetCurrentLayer()->Save(true); }, has_layer});
    file->items.push_front({});
    file->items.push_front({ICON_FA_FOLDER_OPEN " Open", "", [editor]() { DrawModalDialog<OpenUsdFileModalDialog>(*editor); }});
    file->items.push_front({ICON_FA_FILE " New", "", [editor]() { DrawModalDialog<CreateUsdFileModalDialog>(*editor); }});

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
        StageLoader editor;
        HydraRenderer viewport;

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
