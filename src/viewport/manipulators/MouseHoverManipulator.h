#pragma once
#include "Manipulator.h"
class MouseHoverManipulator : public Manipulator {
  public:
    Manipulator *OnUpdate(const pxr::UsdStageRefPtr &stage, std::unique_ptr<pxr::HdSelection> &selection, HydraRenderer &viewport) override;
};
