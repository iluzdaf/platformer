#include <glaze/glaze.hpp>
#include <imgui.h>
#include "ui/game_settings_ui.hpp"
#include "ui/saved_in_scope.hpp"
#include <set>
#include <string>
#include <string_view>
#include <optional>
#include "ui/saveable.hpp"
#include "ui/data_inspector.hpp"
#include "ui/inspector_edited.hpp"
#include "ui/editor_commands.hpp"
#include "game/game_data.hpp"
#include "ui/in_scope.hpp"
#include "ui/sheet_in_scope.hpp"
#include "rendering/texture_cache.hpp"
#include "rendering/texture2d.hpp"

namespace
{
    template <class Icon>
    inspector::Edited drawUnderItsOwnSheet(
        std::string_view name,
        Icon &icon,
        const TextureCache &textures,
        EditorCommands &commands,
        std::set<std::string> &askedToWarm)
    {
        const std::string &path = icon.sheet.texture.path;
        const Texture2D *texture = textures.find(path);
        if (!texture && !path.empty() && askedToWarm.insert(path).second)
            commands.onWarmTexture(path);

        const SheetInScope scope{texture, icon.sheet};
        InScope offering(scope);

        return inspector::draw(name, icon);
    }
}

void GameSettingsUi::draw(
    GameData &gameData,
    const TextureCache &textures,
    EditorCommands &commands)
{
    SavedInScope was(asItWasSaved<GameSettingsData>(saveable.lastSeen("game")));
    inspector::Edited edited =
        inspector::drawFieldsExcept(gameData.settings, {"scoreIcon", "healthIcon"});
    edited |= drawUnderItsOwnSheet(
        "scoreIcon", gameData.settings.scoreIcon, textures, commands, askedToWarm);
    edited |= drawUnderItsOwnSheet(
        "healthIcon", gameData.settings.healthIcon, textures, commands, askedToWarm);

    if (edited.onCommit)
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

std::optional<std::string> GameSettingsUi::cannotSaveBecause(const GameData &gameData) const
{
    const GameSettingsData &settings = gameData.settings;
    if (settings.windowWidth <= 0 || settings.windowHeight <= 0)
        return "a window " + std::to_string(settings.windowWidth) + " by " +
               std::to_string(settings.windowHeight) + " is one nobody can see";

    return std::nullopt;
}

bool GameSettingsUi::unsavedSince(const GameData &gameData)
{
    return saveable.unsavedSince("game", asJson(gameData.settings));
}

bool GameSettingsUi::reloaded(GameData &current, const GameData &onDisk)
{
    return reload(saveable, "game", current.settings, onDisk.settings);
}
