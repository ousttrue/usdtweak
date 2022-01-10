#pragma once
#include "viewport/Selection.h"
#include <pxr/usd/usd/common.h>

class Viewport;

/// Base class for a manipulator.
/// The manipulator can be seen as an editing state part of a FSM
/// It also knows how to draw itself in the viewport

/// This base class forces the override of OnUpdate which is called every frame.
/// The OnUpdate should return the new manipulator to use after the update or itself
/// if the state (Translate/Rotate) should be the same

struct Manipulator {
    virtual ~Manipulator(){};
    virtual void OnBeginEdition(const pxr::UsdStageRefPtr &stage, Viewport &){};     // Enter State
    virtual void OnEndEdition(const pxr::UsdStageRefPtr &stage, Viewport &){};       // Exit State
    virtual Manipulator *OnUpdate(const pxr::UsdStageRefPtr &stage, Selection &selection, Viewport &) = 0; // Next State

    virtual bool IsMouseOver(const pxr::UsdStageRefPtr &stage, const Viewport &) { return false; };

    /// Draw the translate manipulator as seen in the viewport
    virtual void OnDrawFrame(const pxr::UsdStageRefPtr &stage, const Viewport &){};

    /// Called when the viewport changes its selection
    virtual void OnSelectionChange(const pxr::UsdStageRefPtr &stage, Selection &selection, Viewport &){};
};
