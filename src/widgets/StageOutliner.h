#pragma once
#include <pxr/usd/usd/stage.h>
#include "viewport/Selection.h" // TODO: ideally we should have only pxr headers here

PXR_NAMESPACE_USING_DIRECTIVE

// TODO: selected could be multiple Path, we should pass a HdSelection instead
void DrawStageOutliner(UsdStageRefPtr stage, Selection &selectedPaths);
