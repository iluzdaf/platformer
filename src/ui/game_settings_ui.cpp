#include <imgui.h>
#include "ui/game_settings_ui.hpp"
#include "ui/data_inspector.hpp"
#include "ui/editor_commands.hpp"
#include "game/game_data.hpp"

void GameSettingsUi::draw(GameData &gameData, EditorCommands &commands)
{
    bool reverted = saveable.drawControls("game", gameData.settings, saveGameSettings);
    ImGui::Separator();
    if (inspector::drawFields(gameData.settings).onCommit || reverted)
        commands.onSettingsChanged();
}

void GameSettingsUi::valuesReplaced()
{
    saveable.valuesReplaced();
}
