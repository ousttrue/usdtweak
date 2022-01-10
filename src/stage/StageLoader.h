#pragma once
#include "Selection.h"
#include <pxr/usd/usd/stageCache.h>
#include <pxr/usd/sdf/layer.h>
#include <pxr/usd/sdf/primSpec.h>
#include <set>

/// Editor contains the data shared between widgets, like selections, stages, etc etc
class StageLoader {

  public:
    StageLoader();
    ~StageLoader();

    /// Removing the copy constructors as we want to make sure there are no unwanted copies of the
    /// editor. There should be only one editor for now but we want to control the construction
    /// and destruction of the editor to delete properly the contexts, so it's not a singleton
    StageLoader(const StageLoader &) = delete;
    StageLoader &operator=(const StageLoader &) = delete;

    /// Sets the current edited layer
    void SetCurrentLayer(SdfLayerRefPtr layer);
    // void SetCurrentLayerAndPrim(SdfLayerRefPtr layer, SdfPrimSpecHandle sdfPrim);
    SdfLayerRefPtr GetCurrentLayer();
    void SetPreviousLayer(); // go backward in the layer history
    void SetNextLayer();     // go forward in the layer history

    /// List of stages
    /// Using a stage cache to store the stages, seems to work well
    UsdStageRefPtr GetCurrentStage() { return _currentStage; }
    void SetCurrentStage(UsdStageCache::Id current);
    void SetCurrentStage(UsdStageRefPtr stage);

    void SetCurrentEditTarget(SdfLayerHandle layer);

    UsdStageCache &GetStageCache() { return _stageCache; }

    Selection &GetSelection() { return _selection; }

    /// Returns the selected primspec
    /// There should be one selected primspec per layer ideally, so it's very likely this function will move
    SdfPrimSpecHandle &GetSelectedPrimSpec() { return _selectedPrimSpec; }
    void SetSelectedPrimSpec(const SdfPath &primPath);

    /// Create a new layer in file path
    void CreateLayer(const std::string &path);
    void ImportLayer(const std::string &path);
    void CreateStage(const std::string &path);
    void ImportStage(const std::string &path, bool openLoaded = true);
    void SaveCurrentLayerAs(const std::string &path);

  private:
    /// Make sure the layer is correctly in the list of layers,
    /// makes it current and show the appropriate windows
    void UseLayer(SdfLayerRefPtr layer);

    /// Using a stage cache to store the stages, seems to work well
    UsdStageCache _stageCache;

    /// List of layers.
    std::set<SdfLayerRefPtr> _layers;
    SdfLayerRefPtrVector _layerHistory;
    size_t _layerHistoryPointer;

    UsdStageRefPtr _currentStage;

    // Editor owns the selection for the application
    Selection _selection;

    /// Selected prim spec. This variable might move somewhere else
    SdfPrimSpecHandle _selectedPrimSpec;
};
