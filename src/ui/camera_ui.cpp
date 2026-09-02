#include <tuple>
#include <glaze/glaze.hpp>
#include <imgui.h>
#include "ui/camera_ui.hpp"
#include "ui/saveable.hpp"
#include "ui/data_inspector.hpp"
#include "ui/editor_commands.hpp"
#include "cameras/camera2d.hpp"
#include "game/game_data.hpp"

void CameraUi::draw(GameData &gameData, const Camera2D &camera, EditorCommands &commands)
{
    ImGui::TextDisabled("%s", camera.shaking() ? "shaking" : "still");

    ImGui::Separator();
    if (inspector::drawFields(gameData.cameraData))
        commands.onCameraChanged();
}
void CameraUi::revert(GameData &gameData)
{
    std::ignore = glz::read_json(gameData.cameraData, saveable.lastSeen("camera"));
}

void CameraUi::save(GameData &gameData)
{
    saveCameraData(gameData.cameraData);
    saveable.saved("camera", asJson(gameData.cameraData));
}

bool CameraUi::hasUnsavedChanges(const GameData &gameData)
{
    return saveable.unsavedSince("camera", asJson(gameData.cameraData));
}

void CameraUi::valuesReplaced()
{
    saveable.valuesReplaced();
}
