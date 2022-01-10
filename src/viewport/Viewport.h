#pragma once
///
/// OpenGL/Hydra Viewport and its ImGui Window drawing functions.
/// This will eventually be split in 2 different files as the code
/// has grown too much and doing too many thing
///
#include <map>
#include "manipulators/Manipulator.h"
#include "manipulators/CameraManipulator.h"
#include "manipulators/PositionManipulator.h"
#include "manipulators/MouseHoverManipulator.h"
#include "manipulators/SelectionManipulator.h"
#include "manipulators/RotationManipulator.h"
#include "manipulators/ScaleManipulator.h"
#include "Selection.h"
#include "Grid.h"
#include <pxr/imaging/glf/drawTarget.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/camera.h>
#include <pxr/usdImaging/usdImagingGL/engine.h>
#include <pxr/usdImaging/usdImagingGL/renderParams.h>
#include <pxr/imaging/glf/simpleLight.h>

class Viewport final {
  public:
    Viewport();
    ~Viewport();

    // Delete copy
    Viewport(const Viewport &) = delete;
    Viewport &operator=(const Viewport &) = delete;

    /// Render hydra
    void Render(UsdStageRefPtr stage, Selection &selection);

  private:
    /// Update internal data: selection, current renderer
    void Update(UsdStageRefPtr stage, Selection &selection);

  public:
    /// Set the hydra render size
    void SetSize(int width, int height);

    /// Draw the full widget
    void Draw(UsdStageRefPtr stage, Selection &selection);

  private:
    void DrawCameraList(UsdStageRefPtr stage);

  public:
    /// Returns the time code of this viewport
    UsdTimeCode GetCurrentTimeCode() const { return _renderparams ? _renderparams->frame : UsdTimeCode::Default(); }
    void SetCurrentTimeCode(const UsdTimeCode &tc);

    /// Camera framing
    void FrameSelection(const pxr::UsdStageRefPtr &stage, const Selection &);
    void FrameRootPrim(UsdStageRefPtr stage);

    // Cameras
    /// Return the camera used to render the viewport
    GfCamera &GetCurrentCamera();
    const GfCamera &GetCurrentCamera() const;

    // Set the camera path
    void SetCameraPath(const pxr::UsdStageRefPtr &stage, const SdfPath &cameraPath);
    const SdfPath &GetCameraPath() { return _selectedCameraPath; }
    // Returns a UsdGeom camera if the selected camera is in the stage
    UsdGeomCamera GetUsdGeomCamera(const pxr::UsdStageRefPtr &stage);

    CameraManipulator &GetCameraManipulator() { return _cameraManipulator; }

    // Picking
    bool TestIntersection(const pxr::UsdStageRefPtr &stage, GfVec2d clickedPoint, SdfPath &outHitPrimPath,
                          SdfPath &outHitInstancerPath, int &outHitInstanceIndex);
    GfVec2d GetPickingBoundarySize() const;

    // Utility function for compute a scale for the manipulators. It uses the distance between the camera
    // and objectPosition. TODO: remove multiplier, not useful anymore
    double ComputeScaleFactor(const GfVec3d &objectPosition, double multiplier = 1.0) const;

    /// All the manipulators are currently stored in this class, this might change, but right now
    /// GetManipulator is the function that will return the official manipulator based on its type ManipulatorT
    template <typename ManipulatorT> inline Manipulator *GetManipulator();
    Manipulator &GetActiveManipulator() { return *_activeManipulator; }

    // The chosen manipulator is the one selected in the toolbar, Translate/Rotate/Scale/Select ...
    // Manipulator *_chosenManipulator;
    template <typename ManipulatorT> inline void ChooseManipulator() { _activeManipulator = GetManipulator<ManipulatorT>(); };
    template <typename ManipulatorT> inline bool IsChosenManipulator() {
        return _activeManipulator == GetManipulator<ManipulatorT>();
    };

    /// Draw manipulator toolbox, to select translate, rotate, scale
    void DrawManipulatorToolbox(const struct ImVec2 &origin);

    // Position of the mouse in the viewport in normalized unit
    // This is computed in HandleEvents

    GfVec2d GetMousePosition() const { return _mousePosition; }

    SelectionManipulator &GetSelectionManipulator() { return _selectionManipulator; }

    /// Handle events is implemented as a finite state machine.
    /// The state are simply the current manipulator used.
    void HandleManipulationEvents(const pxr::UsdStageRefPtr &stage, Selection &selection);
    void HandleKeyboardShortcut();

  private:
    // GL Lights
    GlfSimpleLightVector _lights;
    GlfSimpleMaterial _material;
    GfVec4f _ambient;

    // Manipulators
    Manipulator *_currentEditingState; // Manipulator currently used by the FSM
    Manipulator *_activeManipulator;   // Manipulator chosen by the user
    CameraManipulator _cameraManipulator;
    PositionManipulator _positionManipulator;
    RotationManipulator _rotationManipulator;
    MouseHoverManipulator _mouseHover;
    ScaleManipulator _scaleManipulator;
    SelectionManipulator _selectionManipulator;

    SelectionHash _lastSelectionHash = 0;

    /// Cameras
    SdfPath _selectedCameraPath;
    GfCamera *_renderCamera; // Points to a valid camera, stage or perspective
    GfCamera _stageCamera;
    // TODO: if we want to have multiple viewport, the persp camera shouldn't belong to the viewport but
    // another shared object, CameraList or similar
    GfCamera _perspectiveCamera; // opengl

    GfVec2i _viewportSize;
    GfVec2d _mousePosition;
    Grid _grid;

    // Renderer
    GLuint _textureId = 0;
    std::map<UsdStageRefPtr, UsdImagingGLEngine *> _renderers;
    UsdImagingGLEngine *_renderer = nullptr;
    UsdImagingGLRenderParams *_renderparams = nullptr;
    GlfDrawTargetRefPtr _drawTarget;
};

template <> inline Manipulator *Viewport::GetManipulator<PositionManipulator>() { return &_positionManipulator; }
template <> inline Manipulator *Viewport::GetManipulator<RotationManipulator>() { return &_rotationManipulator; }
template <> inline Manipulator *Viewport::GetManipulator<MouseHoverManipulator>() { return &_mouseHover; }
template <> inline Manipulator *Viewport::GetManipulator<CameraManipulator>() { return &_cameraManipulator; }
template <> inline Manipulator *Viewport::GetManipulator<SelectionManipulator>() { return &_selectionManipulator; }
template <> inline Manipulator *Viewport::GetManipulator<ScaleManipulator>() { return &_scaleManipulator; }
