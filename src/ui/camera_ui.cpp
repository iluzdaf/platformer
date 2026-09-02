#include <imgui.h>
#include "ui/camera_ui.hpp"
#include "ui/save_controls.hpp"
#include "ui/saveable.hpp"
#include "ui/data_inspector.hpp"
#include "ui/editor_commands.hpp"
#include "cameras/camera2d.hpp"
#include "game/game_data.hpp"

void CameraUi::draw(GameData &gameData, const Camera2D &camera, EditorCommands &commands)
{
    ImGui::TextDisabled("%s", camera.shaking() ? "shaking" : "still");

    ImGui::Separator();
    bool reverted = drawSaveControls(saveable, "camera", gameData.cameraData, saveCameraData);
    ImGui::Separator();
    if (inspector::drawFields(gameData.cameraData) || reverted)
        commands.onCameraChanged();
}
void CameraUi::save(GameData &gameData)
{
    saveCameraData(gameData.cameraData);
    saveable.saved("camera", asJson(gameData.cameraData));
}

bool CameraUi::hasUnsavedChanges(const GameData &gameData) const
{
    return saveable.unsaved("camera", asJson(gameData.cameraData));
}

void CameraUi::valuesReplaced()
{
    saveable.valuesReplaced();
}
