#include <imgui.h>
#include "ui/game_settings_ui.hpp"
#include "ui/save_controls.hpp"
#include "ui/saveable.hpp"
#include "ui/data_inspector.hpp"
#include "ui/editor_commands.hpp"
#include "game/game_data.hpp"

void GameSettingsUi::draw(GameData &gameData, EditorCommands &commands)
{
    bool reverted = drawSaveControls(saveable, "game", gameData.settings, saveGameSettings);
    ImGui::Separator();
    if (inspector::drawFields(gameData.settings).onCommit || reverted)
        commands.onSettingsChanged();
}
void GameSettingsUi::save(GameData &gameData)
{
    saveGameSettings(gameData.settings);
    saveable.saved("game", asJson(gameData.settings));
}

bool GameSettingsUi::hasUnsavedChanges(const GameData &gameData) const
{
    return saveable.unsaved("game", asJson(gameData.settings));
}

void GameSettingsUi::valuesReplaced()
{
    saveable.valuesReplaced();
}
