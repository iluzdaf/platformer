#include <tuple>
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
    std::ignore = glz::read_json(gameData.settings, saveable.lastSeen("game"));
}

void GameSettingsUi::save(GameData &gameData)
{
    saveGameSettings(gameData.settings);
    saveable.saved("game", asJson(gameData.settings));
}

bool GameSettingsUi::hasUnsavedChanges(const GameData &gameData)
{
    return saveable.unsavedSince("game", asJson(gameData.settings));
}

void GameSettingsUi::valuesReplaced()
{
    saveable.valuesReplaced();
}
