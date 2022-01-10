#include "AovUtil.h"
#include <map>

// We keep the currently selected AOV per engine here as there is it not really store in UsdImagingGLEngine.
// When setting a color aov, the engine adds multiple other aov to render, in short there is no easy way to know
// which aov is rendered.
// This is obviously not thread safe but supposed to work in the main rendering thread
static std::map<pxr::TfToken, pxr::TfToken> aovSelection; // a vector might be faster

void SetAovSelection(pxr::UsdImagingGLEngine &renderer, pxr::TfToken aov) {
    aovSelection[renderer.GetCurrentRendererId()] = aov;
}

pxr::TfToken GetAovSelection(pxr::UsdImagingGLEngine &renderer) {
    auto aov = aovSelection.find(renderer.GetCurrentRendererId());
    if (aov == aovSelection.end()) {
        SetAovSelection(renderer, pxr::TfToken("color"));
        return pxr::TfToken("color");
    } else {
        return aov->second;
    }
}

void InitializeRendererAov(pxr::UsdImagingGLEngine &renderer) { renderer.SetRendererAov(GetAovSelection(renderer)); }
