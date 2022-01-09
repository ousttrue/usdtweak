#include "Editor.h"
#include <pxr/imaging/glf/contextCaps.h>
#include <pxr/imaging/garch/glApi.h>
#include <pxr/base/plug/registry.h>
#include <pxr/usd/sdf/fileFormat.h>
#include <pxr/usd/sdf/layer.h>
#include <pxr/usd/sdf/layerUtils.h>
#include <pxr/usd/usd/attribute.h>
#include <pxr/usd/usd/editTarget.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usdGeom/camera.h>
#include <pxr/usd/usdGeom/gprim.h>
#include "commands/Commands.h"
#include "resources/ResourcesLoader.h"
#include <iostream>

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

// /// Modal dialog used to create a new layer
// struct CreateUsdFileModalDialog : public ModalDialog {

//     CreateUsdFileModalDialog(Editor &editor) : editor(editor), createStage(true) { ResetFileBrowserFilePath(); };

//     void Draw() override {
//         DrawFileBrowser();
//         auto filePath = GetFileBrowserFilePath();
//         ImGui::Checkbox("Open as stage", &createStage);
//         if (FilePathExists()) {
//             // ... could add other messages like permission denied, or incorrect extension
//             ImGui::TextColored(ImVec4(1.0f, 0.1f, 0.1f, 1.0f), "Warning: overwriting");
//         } else {
//             if (!filePath.empty()) {
//                 ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "New stage: ");
//             }
//         }

//         ImGui::Text("%s", filePath.c_str());
//         DrawOkCancelModal([&]() {
//             if (!filePath.empty()) {
//                 if (createStage) {
//                     editor.CreateStage(filePath);
//                 } else {
//                     editor.CreateLayer(filePath);
//                 }
//             }
//         });
//     }

//     const char *DialogId() const override { return "Create usd file"; }
//     Editor &editor;
//     bool createStage = true;
// };

// /// Modal dialog to open a layer
// struct OpenUsdFileModalDialog : public ModalDialog {

//     OpenUsdFileModalDialog(Editor &editor) : editor(editor) { SetValidExtensions(GetUsdValidExtensions()); };
//     ~OpenUsdFileModalDialog() override {}
//     void Draw() override {
//         DrawFileBrowser();

//         if (FilePathExists()) {
//             ImGui::Checkbox("Open as stage", &openAsStage);
//             if (openAsStage) {
//                 ImGui::SameLine();
//                 ImGui::Checkbox("Load payloads", &openLoaded);
//             }
//         } else {
//             ImGui::Text("Not found: ");
//         }
//         auto filePath = GetFileBrowserFilePath();
//         ImGui::Text("%s", filePath.c_str());
//         DrawOkCancelModal([&]() {
//             if (!filePath.empty() && FilePathExists()) {
//                 if (openAsStage) {
//                     editor.ImportStage(filePath, openLoaded);
//                 } else {
//                     editor.ImportLayer(filePath);
//                 }
//             }
//         });
//     }

//     const char *DialogId() const override { return "Open layer"; }
//     Editor &editor;
//     bool openAsStage = true;
//     bool openLoaded = true;
// };

// struct SaveLayerAs : public ModalDialog {

//     SaveLayerAs(Editor &editor) : editor(editor){};
//     ~SaveLayerAs() override {}
//     void Draw() override {
//         DrawFileBrowser();
//         auto filePath = GetFileBrowserFilePath();
//         if (FilePathExists()) {
//             ImGui::TextColored(ImVec4(1.0f, 0.1f, 0.1f, 1.0f), "Overwrite: ");
//         } else {
//             ImGui::Text("Save to: ");
//         }
//         ImGui::Text("%s", filePath.c_str());
//         DrawOkCancelModal([&]() { // On Ok ->
//             if (!filePath.empty() && !FilePathExists()) {
//                 editor.SaveCurrentLayerAs(filePath);
//             }
//         });
//     }

//     const char *DialogId() const override { return "Save layer as"; }
//     Editor &editor;
// };

Editor::Editor() : _layerHistoryPointer(0) {

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
}

Editor::~Editor() { SaveSettings(); }

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

bool Editor::HydraRender() {
    if (_shutdownRequested) {
        return false;
    }

#ifndef __APPLE__
    _viewport->Update();
    _viewport->Render();
#endif

    return true;
}

void Editor::SetSelectedPrimSpec(const SdfPath &primPath) {
    if (GetCurrentLayer()) {
        _selectedPrimSpec = GetCurrentLayer()->GetPrimAtPath(primPath);
    }
}

void Editor::LoadSettings() { _settings = ResourcesLoader::GetEditorSettings(); }

void Editor::SaveSettings() const { ResourcesLoader::GetEditorSettings() = _settings; }
