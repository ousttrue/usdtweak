#pragma once

#include "CameraRig.h"
#include "Manipulator.h"
#include <pxr/usd/usdGeom/camera.h>

class CameraManipulator : public CameraRig, public Manipulator {
  public:
    CameraManipulator(const GfVec2i &viewportSize, bool isZUp = false);

    void OnBeginEdition(const pxr::UsdStageRefPtr &stage, HydraRenderer &) override;
    Manipulator *OnUpdate(const pxr::UsdStageRefPtr &stage, Selection &selection, HydraRenderer &) override;
    void OnEndEdition(const pxr::UsdStageRefPtr &stage, HydraRenderer &) override;

  private:
    UsdGeomCamera _stageCamera;
};
