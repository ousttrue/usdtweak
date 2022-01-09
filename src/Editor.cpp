#include <iostream>
#include <array>
#include <utility>
#include <pxr/imaging/glf/contextCaps.h>
#include <pxr/imaging/garch/glApi.h>
#include <pxr/base/plug/registry.h>
#include <pxr/base/arch/fileSystem.h>
#include <pxr/usd/sdf/fileFormat.h>
#include <pxr/usd/sdf/layer.h>
#include <pxr/usd/sdf/layerUtils.h>
#include <pxr/usd/usd/attribute.h>
#include <pxr/usd/usd/editTarget.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usdGeom/camera.h>
#include <pxr/usd/usdGeom/gprim.h>
#include "Gui.h"
#include "Editor.h"
#include "LayerEditor.h"
#include "LayerHierarchyEditor.h"
#include "FileBrowser.h"
#include "PropertyEditor.h"
#include "ModalDialogs.h"
#include "StageOutliner.h"
#include "Timeline.h"
#include "ContentBrowser.h"
#include "PrimSpecEditor.h"
#include "Constants.h"
#include "Commands.h"
#include "ResourcesLoader.h"
#include "TextEditor.h"
#include "Shortcuts.h"
#include <GLFW/glfw3.h>

void Dock::menu_item() { ImGui::MenuItem(_name.c_str(), nullptr, _p_open); }

// There is a bug in the Undo/Redo when reloading certain layers, here is the post
// that explains how to debug the issue:
// Reloading model.stage doesn't work but reloading stage separately does
// https://groups.google.com/u/1/g/usd-interest/c/lRTmWgq78dc/m/HOZ6x9EdCQAJ

// Get usd known file format extensions and returns then prefixed with a dot and in a vector
static const std::vector<std::string> GetUsdValidExtensions() {
    const auto usdExtensions = SdfFileFormat::FindAllFileFormatExtensions();
    std::vector<std::string> validExtensions;
    auto addDot = [](const std::string &str) { return "." + str; };
    std::transform(usdExtensions.cbegin(), usdExtensions.cend(), std::back_inserter(validExtensions), addDot);
    return validExtensions;
}

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

static void BeginBackgoundDock() {
    // Setup dockspace using experimental imgui branch
    static bool alwaysOpened = true;
    static ImGuiDockNodeFlags dockFlags = ImGuiDockNodeFlags_None;
    static ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    windowFlags |=
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace", &alwaysOpened, windowFlags);
    ImGui::PopStyleVar(3);

    ImGuiID dockspaceid = ImGui::GetID("dockspace");
    ImGui::DockSpace(dockspaceid, ImVec2(0.0f, 0.0f), dockFlags);
}

static void EndBackgroundDock() { ImGui::End(); }

/// Call back for dropping a file in the ui
/// TODO Drop callback should popup a modal dialog with the different options available
void Editor::DropCallback(GLFWwindow *window, int count, const char **paths) {
    void *userPointer = glfwGetWindowUserPointer(window);
    if (userPointer) {
        Editor *editor = static_cast<Editor *>(userPointer);
        // TODO: Create a task, add a callback
        if (editor && count) {
            for (int i = 0; i < count; ++i) {
                // make a drop event ?
                if (ArchGetFileLength(paths[i]) == 0) {
                    // if the file is empty, this is considered a new file
                    editor->CreateStage(std::string(paths[i]));
                } else {
                    editor->ImportLayer(std::string(paths[i]));
                }
            }
        }
    }
}

Editor::Editor(GLFWwindow *window) : _layerHistoryPointer(0) {

    // Init glew with USD
    GarchGLApiLoad();
    GlfContextCaps::InitInstance();
    std::cout << glGetString(GL_VENDOR) << std::endl;
    std::cout << glGetString(GL_RENDERER) << std::endl;
    std::cout << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    std::cout << "Hydra enabled : " << UsdImagingGLEngine::IsHydraEnabled() << std::endl;
    // GlfRegisterDefaultDebugOutputMessageCallback();

    _viewport = std::make_shared<Viewport>(UsdStageRefPtr(), _selection);

    ExecuteAfterDraw<EditorSetDataPointer>(this); // This is specialized to execute here, not after the draw
    LoadSettings();

    // Create a context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = nullptr;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    // ImGuiStyle& style = ImGui::GetStyle();
    // style.Colors[ImGuiCol_Tab] = style.Colors[ImGuiCol_FrameBg];
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigWindowsMoveFromTitleBarOnly = true;

    // setup docks
    _docks.push_back(Dock("Stage viewport", &_settings._showViewport, [self = this](bool *p_open) {
        if (ImGui::Begin("Viewport", p_open)) {
            ImVec2 wsize = ImGui::GetWindowSize();
            self->_viewport->SetSize(wsize.x, wsize.y - ViewportBorderSize); // for the next render
            self->_viewport->Draw();
        }
        ImGui::End();
    }));

    _docks.push_back(Dock("Debug window", &_settings._showDebugWindow, [](bool *p_open) {
        if (ImGui::Begin("Debug window", p_open)) {
            // DrawDebugInfo();
            ImGui::Text("\xee\x81\x99"
                        " %.3f ms/frame  (%.1f FPS)",
                        1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        }
        ImGui::End();
    }));

    _docks.push_back(Dock("Stage property editor", &_settings._showPropertyEditor, [self = this](bool *p_open) {
        ImGuiWindowFlags windowFlags = 0 | ImGuiWindowFlags_MenuBar;
        if (ImGui::Begin("Stage property editor", p_open, windowFlags)) {
            if (auto stg = self->GetCurrentStage()) {
                auto prim = stg->GetPrimAtPath(GetSelectedPath(self->_selection));
                DrawUsdPrimProperties(prim, self->_viewport->GetCurrentTimeCode());
            }
        }
        ImGui::End();
    }));

    _docks.push_back(Dock("Stage outliner", &_settings._showOutliner, [self = this](bool *p_open) {
        ImGui::Begin("Stage outliner", p_open);
        DrawStageOutliner(self->GetCurrentStage(), self->_selection);
        ImGui::End();
    }));

    _docks.push_back(Dock("Stage timeline", &_settings._showTimeline, [self = this](bool *p_open) {
        ImGui::Begin("Timeline", p_open);
        UsdTimeCode tc = self->_viewport->GetCurrentTimeCode();
        DrawTimeline(self->GetCurrentStage(), tc);
        self->_viewport->SetCurrentTimeCode(tc);
        ImGui::End();
    }));

    _docks.push_back(Dock("Layer navigator", &_settings._showLayerEditor, [self = this](bool *p_open) {
        const auto &rootLayer = self->GetCurrentLayer();
        const std::string title("Layer navigation" +
                                (rootLayer ? (" - " + rootLayer->GetDisplayName() + (rootLayer->IsDirty() ? " *" : " ")) : "") +
                                "###Layer navigation");
        ImGui::Begin(title.c_str(), p_open);
        DrawLayerNavigation(rootLayer, self->GetSelectedPrimSpec());
        ImGui::End();
    }));

    _docks.push_back(Dock("Layer hierarchy", &_settings._showLayerHierarchyEditor, [self = this](bool *p_open) {
        const auto &rootLayer = self->GetCurrentLayer();
        const std::string title("Layer hierarchy " +
                                (rootLayer ? (" - " + rootLayer->GetDisplayName() + (rootLayer->IsDirty() ? " *" : " ")) : "") +
                                "###Layer hierarchy");
        ImGui::Begin(title.c_str(), p_open);
        DrawLayerPrimHierarchy(rootLayer, self->GetSelectedPrimSpec());
        ImGui::End();
    }));

    _docks.push_back(Dock("Layer stack", &_settings._showLayerStackEditor, [self = this](bool *p_open) {
        const auto &rootLayer = self->GetCurrentLayer();
        const std::string title("Layer stack " +
                                (rootLayer ? (" - " + rootLayer->GetDisplayName() + (rootLayer->IsDirty() ? " *" : " ")) : "") +
                                "###Layer stack");
        ImGui::Begin(title.c_str(), p_open);
        DrawLayerSublayers(rootLayer);
        ImGui::End();
    }));

    _docks.push_back(Dock("Content browser", &_settings._showContentBrowser, [self = this](bool *p_open) {
        ImGuiWindowFlags windowFlags = 0 | ImGuiWindowFlags_MenuBar;
        ImGui::Begin("Content browser", p_open, windowFlags);
        DrawContentBrowser(*self);
        ImGui::End();
    }));

    _docks.push_back(Dock("Layer property editor", &_settings._showPrimSpecEditor, [self = this](bool *p_open) {
        ImGui::Begin("Layer property editor", p_open);
        if (self->GetSelectedPrimSpec()) {
            DrawPrimSpecEditor(self->GetSelectedPrimSpec());
        } else {
            DrawLayerHeader(self->GetCurrentLayer());
        }
        ImGui::End();
    }));

    _docks.push_back(Dock("Layer text editor", &_settings._textEditor, [self = this](bool *p_open) {
        ImGui::Begin("Layer text editor", p_open);
        DrawTextEditor(self->GetCurrentLayer());
        ImGui::End();
    }));
}

Editor::~Editor() {
    // Shutdown imgui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    SaveSettings();
}

void Editor::SetCurrentStage(UsdStageCache::Id current) { SetCurrentStage(_stageCache.Find(current)); }

void Editor::SetCurrentStage(UsdStageRefPtr stage) {
    _currentStage = stage;
    // NOTE: We set the default layer to the current stage root
    // this might have side effects
    if (!GetCurrentLayer() && _currentStage) {
        SetCurrentLayer(_currentStage->GetRootLayer());
    }
    // TODO multiple viewport management
    _viewport->SetCurrentStage(stage);
}

void Editor::SetCurrentLayer(SdfLayerRefPtr layer) {
    if (!layer)
        return;
    if (!_layerHistory.empty()) {
        if (GetCurrentLayer() != layer) {
            if (_layerHistoryPointer < _layerHistory.size() - 1) {
                _layerHistory.resize(_layerHistoryPointer + 1);
            }
            _layerHistory.push_back(layer);
            _layerHistoryPointer = _layerHistory.size() - 1;
        }
    } else {
        _layerHistory.push_back(layer);
        _layerHistoryPointer = _layerHistory.size() - 1;
    }
}

void Editor::SetCurrentEditTarget(SdfLayerHandle layer) {
    if (GetCurrentStage()) {
        GetCurrentStage()->SetEditTarget(UsdEditTarget(layer));
    }
}

SdfLayerRefPtr Editor::GetCurrentLayer() {
    return _layerHistory.empty() ? SdfLayerRefPtr() : _layerHistory[_layerHistoryPointer];
}

void Editor::SetPreviousLayer() {
    if (_layerHistoryPointer > 0) {
        _layerHistoryPointer--;
    }
}

void Editor::SetNextLayer() {
    if (_layerHistoryPointer < _layerHistory.size() - 1) {
        _layerHistoryPointer++;
    }
}

void Editor::UseLayer(SdfLayerRefPtr layer) {
    if (layer) {
        if (_layers.find(layer) == _layers.end()) {
            _layers.emplace(layer);
        }
        SetCurrentLayer(layer);
        _settings._showContentBrowser = true;
        _settings._showLayerEditor = true;
    }
}

void Editor::CreateLayer(const std::string &path) {
    auto newLayer = SdfLayer::CreateNew(path);
    UseLayer(newLayer);
}

void Editor::ImportLayer(const std::string &path) {
    auto newLayer = SdfLayer::FindOrOpen(path);
    UseLayer(newLayer);
}

//
void Editor::ImportStage(const std::string &path, bool openLoaded) {
    auto newStage = UsdStage::Open(path, openLoaded ? UsdStage::LoadAll : UsdStage::LoadNone); // TODO: as an option
    if (newStage) {
        _stageCache.Insert(newStage);
        SetCurrentStage(newStage);
        _settings._showContentBrowser = true;
        _settings._showViewport = true;
    }
}

void Editor::SaveCurrentLayerAs(const std::string &path) {
    auto newLayer = SdfLayer::CreateNew(path);
    if (newLayer && GetCurrentLayer()) {
        newLayer->TransferContent(GetCurrentLayer());
        newLayer->Save();
        UseLayer(newLayer);
    }
}

void Editor::CreateStage(const std::string &path) {
    auto usdaFormat = SdfFileFormat::FindByExtension("usda");
    auto layer = SdfLayer::New(usdaFormat, path);
    if (layer) {
        auto newStage = UsdStage::Open(layer);
        if (newStage) {
            _stageCache.Insert(newStage);
            SetCurrentStage(newStage);
            _settings._showContentBrowser = true;
            _settings._showViewport = true;
        }
    }
}

void Editor::HydraRender() {
#ifndef __APPLE__
    _viewport->Update();
    _viewport->Render();
#endif
}

void Editor::DrawMainMenuBar() {

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem(ICON_FA_FILE " New")) {
                DrawModalDialog<CreateUsdFileModalDialog>(*this);
            }
            if (ImGui::MenuItem(ICON_FA_FOLDER_OPEN " Open")) {
                DrawModalDialog<OpenUsdFileModalDialog>(*this);
            }
            ImGui::Separator();
            const bool hasLayer = GetCurrentLayer() != SdfLayerRefPtr();
            if (ImGui::MenuItem(ICON_FA_SAVE " Save layer", "CTRL+S", false, hasLayer)) {
                GetCurrentLayer()->Save(true);
            }
            if (ImGui::MenuItem(ICON_FA_SAVE " Save layer as", "CTRL+F", false, hasLayer)) {
                DrawModalDialog<SaveLayerAs>(*this);
            }

            ImGui::Separator();
            if (ImGui::MenuItem("Quit")) {
                _shutdownRequested = true;
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "CTRL+Z")) {
                ExecuteAfterDraw<UndoCommand>();
            }
            if (ImGui::MenuItem("Redo", "CTRL+R")) {
                ExecuteAfterDraw<RedoCommand>();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", "CTRL+X", false, false)) {
            }
            if (ImGui::MenuItem("Copy", "CTRL+C", false, false)) {
            }
            if (ImGui::MenuItem("Paste", "CTRL+V", false, false)) {
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Windows")) {
            for (auto &dock : _docks) {
                dock.menu_item();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
}

void Editor::SetSelectedPrimSpec(const SdfPath &primPath) {
    if (GetCurrentLayer()) {
        _selectedPrimSpec = GetCurrentLayer()->GetPrimAtPath(primPath);
    }
}

void Editor::LoadSettings() { _settings = ResourcesLoader::GetEditorSettings(); }

void Editor::SaveSettings() const { ResourcesLoader::GetEditorSettings() = _settings; }

bool Editor::Update() {
    if (_shutdownRequested) {
        return false;
    }

    // Render the viewports first as textures
    HydraRender();

    // imgui
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    {
        // Dock
        BeginBackgoundDock();

        // Main Menu bar
        DrawMainMenuBar();

        for (auto &dock : _docks) {
            dock.show();
        }

        DrawCurrentModal();

        ///////////////////////
        // Top level shortcuts functions
        AddShortcut<UndoCommand, GLFW_KEY_LEFT_CONTROL, GLFW_KEY_Z>();
        AddShortcut<RedoCommand, GLFW_KEY_LEFT_CONTROL, GLFW_KEY_R>();

        EndBackgroundDock();
    }
    ImGui::Render();

    return true;
}

void Editor::Render() { ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); }
