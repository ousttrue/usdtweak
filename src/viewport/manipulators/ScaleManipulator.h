#pragma once
#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec2d.h>
#include <pxr/base/gf/line.h>
#include <pxr/usd/usdGeom/gprim.h>
#include <pxr/usd/usdGeom/xformCommonAPI.h>

#include "Manipulator.h"

PXR_NAMESPACE_USING_DIRECTIVE

// Scale manipulator
// This is a almost perfect copy of the position manipulator, this needs to be factored in

class ScaleManipulator : public Manipulator {

  public:
    ScaleManipulator();
    ~ScaleManipulator();

    void OnBeginEdition(const pxr::UsdStageRefPtr &stage, HydraRenderer &) override;
    Manipulator *OnUpdate(const pxr::UsdStageRefPtr &stage, std::unique_ptr<pxr::HdSelection> &selection, HydraRenderer &) override;
    void OnEndEdition(const pxr::UsdStageRefPtr &stage, HydraRenderer &) override;

    /// Return true if the mouse is over this manipulator for the viewport passed in argument
    bool IsMouseOver(const pxr::UsdStageRefPtr &stage, const HydraRenderer &) override;

    /// Draw the translate manipulator as seen in the viewport
    void OnDrawFrame(const pxr::UsdStageRefPtr &stage, const HydraRenderer &) override;

    /// Called when the viewport changes its selection
    void OnSelectionChange(const pxr::UsdStageRefPtr &stage, std::unique_ptr<pxr::HdSelection> &selection, HydraRenderer &) override;

    typedef enum { // use class enum ??
        XAxis = 0,
        YAxis,
        ZAxis,
        None,
    } SelectedAxis;

  private:
    bool CompileShaders();

    void ProjectMouseOnAxis(const HydraRenderer &viewport, GfVec3d &closestPoint);
    GfMatrix4d ComputeManipulatorToWorldTransform(const HydraRenderer &viewport);

    UsdTimeCode GetEditionTimeCode(const HydraRenderer &viewport);

    SelectedAxis _selectedAxis;

    GfVec3d _originMouseOnAxis;
    GfVec3f _scaleOnBegin;
    GfLine _axisLine;

    UsdGeomXformCommonAPI _xformAPI;

    // OpenGL identifiers
    unsigned int _axisGLBuffers;
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
