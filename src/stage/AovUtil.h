#pragma once
#include <pxr/usdImaging/usdImagingGL/engine.h>

/// We keep track of the selected AOV in the UI, unfortunately the selected AOV is not awvailable in
/// UsdImagingGLEngine, so we need the initialize the UI data with this function
void InitializeRendererAov(pxr::UsdImagingGLEngine &);
void SetAovSelection(pxr::UsdImagingGLEngine &renderer, pxr::TfToken aov);
pxr::TfToken GetAovSelection(pxr::UsdImagingGLEngine &renderer);
