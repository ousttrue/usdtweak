#pragma once
#include <pxr/usd/usd/stage.h>
#include <pxr/imaging/hd/selection.h>
#include <memory>

// TODO: selected could be multiple Path, we should pass a HdSelection instead
void DrawStageOutliner(pxr::UsdStageRefPtr stage, std::unique_ptr<pxr::HdSelection> &selectedPaths);
