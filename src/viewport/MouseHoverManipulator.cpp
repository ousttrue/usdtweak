#include "MouseHoverManipulator.h"
#include "Viewport.h"
#include "Gui.h"

Manipulator *MouseHoverManipulator::OnUpdate(Viewport &viewport) {
    ImGuiIO &io = ImGui::GetIO();

    if (io.KeyAlt) {
        return viewport.GetManipulator<CameraManipulator>();
    } else if (ImGui::IsMouseClicked(0)) {
        auto &manipulator = viewport.GetActiveManipulator();
        if (manipulator.IsMouseOver(viewport)) {
            return &manipulator;
        } else {
            return viewport.GetManipulator<SelectionManipulator>();
        }
    } else if (ImGui::IsKeyPressed('F')) {
        const Selection &selection = viewport.GetSelection();
        if (!IsSelectionEmpty(selection)) {
            viewport.FrameSelection(viewport.GetSelection());
        } else {
            viewport.FrameRootPrim();
        }
    } else {
        auto &manipulator = viewport.GetActiveManipulator();
        manipulator.IsMouseOver(viewport);
    }
    return this;
}