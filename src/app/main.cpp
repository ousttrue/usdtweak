#include <Dockspace.h>
#include "CommandLineOptions.h"
#include "window.h"
#include <StageLoader.h>
#include <commands/Commands.h>
#include <HydraRenderer.h>
#include <Constants.h>
//
#include <PropertyEditor.h>
#include <StageOutliner.h>
#include <Timeline.h>
#include <LayerEditor.h>
#include <LayerHierarchyEditor.h>
#include <LayerEditor.h>
#include <ContentBrowser.h>
#include <PrimSpecEditor.h>
#include <LayerEditor.h>
#include <TextEditor.h>
#include <ModalDialogs.h>
#include <FileBrowser.h>
//
#include <pxr/base/arch/fileSystem.h>
#include <pxr/usd/sdf/fileFormat.h>
#include <imgui.h>
#include <IconsFontAwesome5.h>
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

/// Viewport border between the panel and the window
constexpr float ViewportBorderSize = 60.f;

// setup docks
void Setup(Dockspace *dockspace, StageLoader *loader, HydraRenderer *viewport) {
    auto &_docks = dockspace->Docks();

    _docks.push_back(Dock("Stage viewport", &dockspace->Settings()._showViewport, [loader, viewport](bool *p_open) {
        if (ImGui::Begin("Viewport", p_open)) {
            ImVec2 wsize = ImGui::GetWindowSize();
            viewport->Draw(loader->GetCurrentStage(), loader->GetSelection(), static_cast<int>(wsize.x),
                           static_cast<int>(wsize.y - ViewportBorderSize));
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

    _docks.push_back(Dock("Stage property editor", &dockspace->Settings()._showPropertyEditor, [loader, viewport](bool *p_open) {
        ImGuiWindowFlags windowFlags = 0 | ImGuiWindowFlags_MenuBar;
        if (ImGui::Begin("Stage property editor", p_open, windowFlags)) {
            if (auto stg = loader->GetCurrentStage()) {
                auto prim = stg->GetPrimAtPath(GetSelectedPath(loader->GetSelection()));
                DrawUsdPrimProperties(prim, viewport->GetCurrentTimeCode());
            }
        }
        ImGui::End();
    }));

    _docks.push_back(Dock("Stage outliner", &dockspace->Settings()._showOutliner, [loader, viewport](bool *p_open) {
        ImGui::Begin("Stage outliner", p_open);
        DrawStageOutliner(loader->GetCurrentStage(), loader->GetSelection());
        ImGui::End();
    }));

    _docks.push_back(Dock("Stage timeline", &dockspace->Settings()._showTimeline, [loader, viewport](bool *p_open) {
        ImGui::Begin("Timeline", p_open);
        UsdTimeCode tc = viewport->GetCurrentTimeCode();
        DrawTimeline(loader->GetCurrentStage(), tc);
        viewport->SetCurrentTimeCode(tc);
        ImGui::End();
    }));

    _docks.push_back(Dock("Layer navigator", &dockspace->Settings()._showLayerEditor, [loader](bool *p_open) {
        const auto &rootLayer = loader->GetCurrentLayer();
        const std::string title("Layer navigation" +
                                (rootLayer ? (" - " + rootLayer->GetDisplayName() + (rootLayer->IsDirty() ? " *" : " ")) : "") +
                                "###Layer navigation");
        ImGui::Begin(title.c_str(), p_open);
        DrawLayerNavigation(rootLayer, loader->GetSelectedPrimSpec());
        ImGui::End();
    }));

    _docks.push_back(Dock("Layer hierarchy", &dockspace->Settings()._showLayerHierarchyEditor, [loader](bool *p_open) {
        const auto &rootLayer = loader->GetCurrentLayer();
        const std::string title("Layer hierarchy " +
                                (rootLayer ? (" - " + rootLayer->GetDisplayName() + (rootLayer->IsDirty() ? " *" : " ")) : "") +
                                "###Layer hierarchy");
        ImGui::Begin(title.c_str(), p_open);
        DrawLayerPrimHierarchy(rootLayer, loader->GetSelectedPrimSpec());
        ImGui::End();
    }));

    _docks.push_back(Dock("Layer stack", &dockspace->Settings()._showLayerStackEditor, [loader](bool *p_open) {
        const auto &rootLayer = loader->GetCurrentLayer();
        const std::string title("Layer stack " +
                                (rootLayer ? (" - " + rootLayer->GetDisplayName() + (rootLayer->IsDirty() ? " *" : " ")) : "") +
                                "###Layer stack");
        ImGui::Begin(title.c_str(), p_open);
        DrawLayerSublayers(rootLayer);
        ImGui::End();
    }));

    _docks.push_back(Dock("Content browser", &dockspace->Settings()._showContentBrowser, [loader](bool *p_open) {
        ImGuiWindowFlags windowFlags = 0 | ImGuiWindowFlags_MenuBar;
        ImGui::Begin("Content browser", p_open, windowFlags);
        DrawContentBrowser(*loader);
        ImGui::End();
    }));

    _docks.push_back(Dock("Layer property editor", &dockspace->Settings()._showPrimSpecEditor, [loader](bool *p_open) {
        ImGui::Begin("Layer property editor", p_open);
        if (loader->GetSelectedPrimSpec()) {
            DrawPrimSpecEditor(loader->GetSelectedPrimSpec());
        } else {
            DrawLayerHeader(loader->GetCurrentLayer());
        }
        ImGui::End();
    }));

    _docks.push_back(Dock("Layer text editor", &dockspace->Settings()._textEditor, [loader](bool *p_open) {
        ImGui::Begin("Layer text editor", p_open);
        DrawTextEditor(loader->GetCurrentLayer());
        ImGui::End();
    }));

    auto file = dockspace->GetMenu("File");
    auto has_layer = [loader]() { return loader->GetCurrentLayer() != SdfLayerRefPtr(); };
    file->items.push_front(
        {ICON_FA_SAVE " Save layer as", "CTRL+F", [loader]() { DrawModalDialog<SaveLayerAs>(*loader); }, has_layer});
    file->items.push_front(
        {ICON_FA_SAVE " Save layer", "CTRL+S", [loader]() { loader->GetCurrentLayer()->Save(true); }, has_layer});
    file->items.push_front({});
    file->items.push_front({ICON_FA_FOLDER_OPEN " Open", "", [loader]() { DrawModalDialog<OpenUsdFileModalDialog>(*loader); }});
    file->items.push_front({ICON_FA_FILE " New", "", [loader]() { DrawModalDialog<CreateUsdFileModalDialog>(*loader); }});

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
    auto window = Window::create(1600, 1200);
    if (!window) {
        return 1;
    }

    Dockspace dockspace(window->get());

    { // Scope as the editor should be deleted before imgui and glfw, to release correctly the memory
        // Resource will load the font/textures/settings
        StageLoader loader;
        HydraRenderer hydra;

        Setup(&dockspace, &loader, &hydra);

        window->setOnDrop([&loader](int count, const char **paths) {
            for (int i = 0; i < count; ++i) {
                // make a drop event ?
                if (ArchGetFileLength(paths[i]) == 0) {
                    // if the file is empty, this is considered a new file
                    loader.CreateStage(std::string(paths[i]));
                } else {
                    loader.ImportLayer(std::string(paths[i]));
                }
            }
        });

        // Process command line options
        for (auto &stage : options.stages()) {
            loader.ImportStage(stage);
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
