#include "CameraManipulator.h"
#include "viewport/Viewport.h"
#include <pxr/usd/usdGeom/camera.h>
#include "commands/Commands.h"
#include <imgui.h>

CameraManipulator::CameraManipulator(const GfVec2i &viewportSize, bool isZUp) : CameraRig(viewportSize, isZUp) {}

void CameraManipulator::OnBeginEdition(const pxr::UsdStageRefPtr &stage, Viewport &viewport) {
    _stageCamera = viewport.GetUsdGeomCamera(stage);
    if (_stageCamera) {
        BeginEdition(stage);
    }
}

void CameraManipulator::OnEndEdition(const pxr::UsdStageRefPtr &stage, Viewport &viewport) {
    if (_stageCamera) {
        EndEdition();
    }
}

Manipulator *CameraManipulator::OnUpdate(const pxr::UsdStageRefPtr &stage, Viewport &viewport) {
    auto &cameraManipulator = viewport.GetCameraManipulator();
    ImGuiIO &io = ImGui::GetIO();

    /// If the user released key alt, escape camera manipulation
    if (!io.KeyAlt) {
        return viewport.GetManipulator<MouseHoverManipulator>();
    } else if (ImGui::IsMouseReleased(1) || ImGui::IsMouseReleased(2) || ImGui::IsMouseReleased(0)) {
        SetMovementType(MovementType::None);
    } else if (ImGui::IsMouseClicked(0)) {
        SetMovementType(MovementType::Orbit);
    } else if (ImGui::IsMouseClicked(2)) {
        SetMovementType(MovementType::Truck);
    } else if (ImGui::IsMouseClicked(1)) {
        SetMovementType(MovementType::Dolly);
    }
    auto &currentCamera = viewport.GetCurrentCamera();

    if (Move(currentCamera, io.MouseDelta.x, io.MouseDelta.y)) {
        if (_stageCamera) {
            // This is going to fill the undo/redo buffer :S
            _stageCamera.SetFromCamera(currentCamera, viewport.GetCurrentTimeCode());
        }
    }

    return this;
}