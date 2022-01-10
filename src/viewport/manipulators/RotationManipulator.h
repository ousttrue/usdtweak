#pragma once
#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/matrix4f.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec2d.h>
#include <pxr/usd/usdGeom/gprim.h>
#include <pxr/usd/usdGeom/xformCommonAPI.h>
#include "Manipulator.h"

PXR_NAMESPACE_USING_DIRECTIVE

class RotationManipulator : public Manipulator {

  public:
    RotationManipulator();
    ~RotationManipulator();

    /// From ViewportEditor
    void OnBeginEdition(const pxr::UsdStageRefPtr &stage, HydraRenderer &) override;
    Manipulator *OnUpdate(const pxr::UsdStageRefPtr &stage, Selection &selection, HydraRenderer &) override;
    void OnEndEdition(const pxr::UsdStageRefPtr &stage, HydraRenderer &) override;

    /// Return true if the mouse is over this manipulator in the viewport passed in argument
    bool IsMouseOver(const pxr::UsdStageRefPtr &stage, const HydraRenderer &) override;

    /// Draw the translate manipulator as seen in the viewport
    void OnDrawFrame(const pxr::UsdStageRefPtr &stage, const HydraRenderer &) override;

    /// Called when the viewport changes its selection
    void OnSelectionChange(const pxr::UsdStageRefPtr &stage, Selection &selection, HydraRenderer &) override;

    typedef enum { // use class enum ??
        XAxis = 0,
        YAxis,
        ZAxis,
        None,
    } SelectedAxis;

  private:
    UsdTimeCode GetEditionTimeCode(const HydraRenderer &);
    UsdTimeCode GetViewportTimeCode(const HydraRenderer &);

    bool CompileShaders();
    GfVec3d ComputeClockHandVector(HydraRenderer &viewport);

    GfMatrix4d ComputeManipulatorToWorldTransform(const HydraRenderer &viewport);
    SelectedAxis _selectedAxis;

    UsdGeomXformCommonAPI _xformAPI;

    GfVec3d _rotateFrom;
    GfMatrix4d _rotateMatrixOnBegin;

    GfVec3d _planeOrigin3d; // Global
    GfVec3d _planeNormal3d; // TODO rename global
    GfVec3d _localPlaneNormal; // Local

    // OpenGL stuff
    unsigned int _arrayBuffer;
    unsigned int _vertexShader;
    unsigned int _fragmentShader;
    unsigned int _programShader;
    unsigned int _vertexArrayObject;
    unsigned int _modelViewUniform;
    unsigned int _projectionUniform;
    unsigned int _scaleFactorUniform;
    unsigned int _objectMatrixUniform;
    unsigned int _highlightUniform;
};
