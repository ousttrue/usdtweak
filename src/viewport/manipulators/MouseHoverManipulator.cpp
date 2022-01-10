#include "MouseHoverManipulator.h"
#include "viewport/Selection.h"
#include "viewport/Viewport.h"
#include <imgui.h>

Manipulator *MouseHoverManipulator::OnUpdate(const pxr::UsdStageRefPtr &stage, Selection &selection, Viewport &viewport) {
    ImGuiIO &io = ImGui::GetIO();

    if (io.KeyAlt) {
        return viewport.GetManipulator<CameraManipulator>();
    } else if (ImGui::IsMouseClicked(0)) {
        auto &manipulator = viewport.GetActiveManipulator();
        if (manipulator.IsMouseOver(stage, viewport)) {
            return &manipulator;
        } else {
            return viewport.GetManipulator<SelectionManipulator>();
        }
    } else if (ImGui::IsKeyPressed('F')) {
        if (!IsSelectionEmpty(selection)) {
            viewport.FrameSelection(stage, selection);
        } else {
            viewport.FrameRootPrim(stage);
        }
    } else {
        auto &manipulator = viewport.GetActiveManipulator();
        manipulator.IsMouseOver(stage, viewport);
    }
    return this;
}