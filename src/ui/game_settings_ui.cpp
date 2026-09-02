#include <glaze/glaze.hpp>
#include <imgui.h>
#include "ui/game_settings_ui.hpp"
#include "ui/saveable.hpp"
#include "ui/data_inspector.hpp"
#include "ui/editor_commands.hpp"
#include "game/game_data.hpp"

void GameSettingsUi::draw(GameData &gameData, EditorCommands &commands)
{
    if (inspector::drawFields(gameData.settings).onCommit)
        commands.onSettingsChanged();
}
void GameSettingsUi::revert(GameData &gameData)
{
    revertTo(saveable, "game", gameData.settings);
}

void GameSettingsUi::save(GameData &gameData)
{
    saveGameSettings(gameData.settings);
    saveable.saved("game", asJson(gameData.settings));
}

bool GameSettingsUi::unsavedSince(const GameData &gameData)
{
    return saveable.unsavedSince("game", asJson(gameData.settings));
}

void GameSettingsUi::valuesReplaced()
{
    saveable.valuesReplaced();
}
