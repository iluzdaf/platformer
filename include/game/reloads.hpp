#pragma once

#include <string>

class World;
class EditorUi;
struct GameData;

namespace reloads
{
    void levelChanged(World &world, EditorUi &editorUi, const std::string &levelPath);

    void gameDataChanged(
        World &world,
        EditorUi &editorUi,
        GameData &gameData,
        const GameData &onDisk);
}
