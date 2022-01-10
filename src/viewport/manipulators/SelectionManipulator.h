#pragma once

#include "Manipulator.h"

/// The selection manipulator will help selecting a region of the viewport, drawing a rectangle.
class SelectionManipulator : public Manipulator {
  public:
    SelectionManipulator() = default;
    ~SelectionManipulator() = default;

    void OnDrawFrame(const pxr::UsdStageRefPtr &stage, const HydraRenderer &) override;

    Manipulator *OnUpdate(const pxr::UsdStageRefPtr &stage, std::unique_ptr<pxr::HdSelection> &selection, HydraRenderer &) override;

    // Picking modes
    enum class PickMode { Prim, Model, Assembly };
    void SetPickMode(PickMode pickMode) { _pickMode = pickMode; }
    PickMode GetPickMode() const { return _pickMode; }

  private:
    // Returns true
    bool IsPickablePath(const pxr::UsdStage &stage, const pxr::SdfPath &path);
    PickMode _pickMode = PickMode::Prim;
};

/// Draw an ImGui menu to select the picking mode
void DrawPickMode(SelectionManipulator &manipulator);
