#pragma once
#include "Manipulator.h"
class MouseHoverManipulator : public Manipulator {
  public:
    Manipulator *OnUpdate(const pxr::UsdStageRefPtr &stage, Viewport &viewport) override;
};
