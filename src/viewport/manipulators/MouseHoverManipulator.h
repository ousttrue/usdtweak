#pragma once
#include "Manipulator.h"
class MouseHoverManipulator : public Manipulator {
  public:
    Manipulator *OnUpdate(const pxr::UsdStageRefPtr &stage, Selection &selection, HydraRenderer &viewport) override;
};
