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

    void OnBeginEdition(const pxr::UsdStageRefPtr &stage, Viewport &) override;
    Manipulator *OnUpdate(const pxr::UsdStageRefPtr &stage, Viewport &) override;
    void OnEndEdition(const pxr::UsdStageRefPtr &stage, Viewport &) override;

    /// Return true if the mouse is over this manipulator for the viewport passed in argument
    bool IsMouseOver(const pxr::UsdStageRefPtr &stage, const Viewport &) override;

    /// Draw the translate manipulator as seen in the viewport
    void OnDrawFrame(const pxr::UsdStageRefPtr &stage, const Viewport &) override;

    /// Called when the viewport changes its selection
    void OnSelectionChange(const pxr::UsdStageRefPtr &stage, Viewport &) override;

    typedef enum { // use class enum ??
        XAxis = 0,
        YAxis,
        ZAxis,
        None,
    } SelectedAxis;

  private:
    bool CompileShaders();

    void ProjectMouseOnAxis(const Viewport &viewport, GfVec3d &closestPoint);
    GfMatrix4d ComputeManipulatorToWorldTransform(const Viewport &viewport);

    UsdTimeCode GetEditionTimeCode(const Viewport &viewport);

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
