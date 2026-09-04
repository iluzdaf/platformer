#include <string>
#include "game/reloads.hpp"
#include "game/game_data.hpp"
#include "game/level_data.hpp"
#include "game/world.hpp"
#include "ui/editor_ui.hpp"

void reloads::levelChanged(World &world, EditorUi &editorUi, const std::string &levelPath)
{
    if (editorUi.levelFollowsTheDisk(world.getLevelData(), levelPath))
        world.loadLevel(levelPath);
}

void reloads::gameDataChanged(
    World &world,
    EditorUi &editorUi,
    GameData &gameData,
    const GameData &onDisk)
{
    editorUi.reloaded(gameData, onDisk);

    std::string current = world.getLevelPath();
    if (current.empty())
        world.loadLevel(gameData.levels.first);
    else if (editorUi.levelFollowsTheDisk(world.getLevelData(), current))
        world.loadLevel(current);
    else
        world.rebuildFrom(LevelData(world.getLevelData()));
}
