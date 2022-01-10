#include "Editor.h"
#include <pxr/imaging/glf/contextCaps.h>
#include <pxr/usd/sdf/fileFormat.h>
#include "commands/Commands.h"
#include <iostream>

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

    ExecuteAfterDraw<EditorSetDataPointer>(this); // This is specialized to execute here, not after the draw
}

Editor::~Editor() {}

void Editor::SetCurrentStage(UsdStageCache::Id current) { SetCurrentStage(_stageCache.Find(current)); }

void Editor::SetCurrentStage(UsdStageRefPtr stage) {
    _currentStage = stage;
    // NOTE: We set the default layer to the current stage root
    // this might have side effects
    if (!GetCurrentLayer() && _currentStage) {
        SetCurrentLayer(_currentStage->GetRootLayer());
    }
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
        // _settings._showContentBrowser = true;
        // _settings._showLayerEditor = true;
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
        // _settings._showContentBrowser = true;
        // _settings._showViewport = true;
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
            // _settings._showContentBrowser = true;
            // _settings._showViewport = true;
        }
    }
}

void Editor::SetSelectedPrimSpec(const SdfPath &primPath) {
    if (GetCurrentLayer()) {
        _selectedPrimSpec = GetCurrentLayer()->GetPrimAtPath(primPath);
    }
}
