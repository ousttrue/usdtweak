#include <iostream>
#include "RendererSettings.h"
#include "Gui.h"



void DrawRendererSettings(UsdImagingGLEngine &renderer, UsdImagingGLRenderParams &renderparams) {
    // General render parameters
    ImGui::ColorEdit4("Background color", renderparams.clearColor.data());

    // TODO: check there isn't a function that already returns the strings for an enum
    // I have seen that in the USD code
    static std::array<std::pair<UsdImagingGLDrawMode, const char *>, 8> UsdImagingGLDrawModeStrings = {
        std::make_pair(UsdImagingGLDrawMode::DRAW_POINTS, "DRAW_POINTS"),
        std::make_pair(UsdImagingGLDrawMode::DRAW_WIREFRAME, "DRAW_WIREFRAME"),
        std::make_pair(UsdImagingGLDrawMode::DRAW_WIREFRAME_ON_SURFACE, "DRAW_WIREFRAME_ON_SURFACE"),
        std::make_pair(UsdImagingGLDrawMode::DRAW_SHADED_FLAT, "DRAW_SHADED_FLAT"),
        std::make_pair(UsdImagingGLDrawMode::DRAW_SHADED_SMOOTH, "DRAW_SHADED_SMOOTH"),
        std::make_pair(UsdImagingGLDrawMode::DRAW_GEOM_ONLY, "DRAW_GEOM_ONLY"),
        std::make_pair(UsdImagingGLDrawMode::DRAW_GEOM_FLAT, "DRAW_GEOM_FLAT"),
        std::make_pair(UsdImagingGLDrawMode::DRAW_GEOM_SMOOTH, "DRAW_GEOM_SMOOTH") };

    // Look for the current on
    auto currentdrawmode = UsdImagingGLDrawModeStrings[0];
    for (const auto &mode : UsdImagingGLDrawModeStrings) {
        if (renderparams.drawMode == mode.first) {
            currentdrawmode = mode;
        }
    }
    if (ImGui::BeginCombo("Draw mode", currentdrawmode.second)) {
        for (const auto &drawMode : UsdImagingGLDrawModeStrings) {
            bool is_selected = (renderparams.drawMode == drawMode.first);
            if (ImGui::Selectable(drawMode.second, is_selected)) {
                renderparams.drawMode = drawMode.first;
            }
        }
        ImGui::EndCombo();
    }

    ImGui::Checkbox("Enable lighting", &renderparams.enableLighting);
    ImGui::Checkbox("Enable scene materials", &renderparams.enableSceneMaterials);
    ImGui::Checkbox("Enable ID render", &renderparams.enableIdRender);

    // Renderer
    ImGui::Separator();
    const auto currentPlugin = renderer.GetCurrentRendererId();
    if (ImGui::BeginCombo("Renderer", currentPlugin.GetText())) {
        auto plugins = renderer.GetRendererPlugins();
        for (int n = 0; n < plugins.size(); n++) {
            bool is_selected = (currentPlugin == plugins[n]);
            if (ImGui::Selectable(plugins[n].GetText(), is_selected)) {
                if (!renderer.SetRendererPlugin(plugins[n])) {
                    std::cerr << "unable to set default renderer plugin" << std::endl;
                }
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    // Renderer settings
    for (auto setting : renderer.GetRendererSettingsList()) {
        // TODO: create a dedicated interface for each parameter
        ImGui::Text("%s", setting.name.c_str());
    }
}