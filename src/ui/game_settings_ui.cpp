#include <glaze/glaze.hpp>
#include <imgui.h>
#include "ui/game_settings_ui.hpp"
#include "ui/saveable.hpp"
#include "ui/data_inspector.hpp"
#include "ui/editor_commands.hpp"
#include "game/game_data.hpp"
#include "game/score_icon_data.hpp"
#include "assets/sheet_data.hpp"
#include "ui/sheet_in_scope.hpp"
#include "rendering/texture_cache.hpp"
#include "rendering/texture2d.hpp"

void GameSettingsUi::draw(
    GameData &gameData,
    const TextureCache &textures,
    EditorCommands &commands)
{
    const SheetData &sheet = gameData.settings.scoreIcon.sheet;
    const Texture2D *texture = textures.find(sheet.texture.path);
    if (!texture && !sheet.texture.path.empty() && sheet.texture.path != askedToWarm)
    {
        askedToWarm = sheet.texture.path;
        commands.onWarmTexture(sheet.texture.path);
    }

    ShowingSheet offering(SheetInScope{texture, sheet});

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

bool GameSettingsUi::reloaded(GameData &current, const GameData &onDisk)
{
    return reload(saveable, "game", current.settings, onDisk.settings);
}
