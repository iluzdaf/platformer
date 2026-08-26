#pragma once

#include <signals.hpp>
#include <string>

class ImGuiManager;
class TileMap;
class Level;
class Texture2D;
class Camera2D;

class LevelEditorUi
{
public:
    void draw(
        const ImGuiManager &imGuiManager,
        Level &level,
        const Texture2D &tileSet,
        bool shouldDrawLevelEditor);
    void update(
        const ImGuiManager &imGuiManager,
        const Camera2D &camera,
        Level &level);

    fteng::signal<void(const std::string &)> onLoadLevel;

private:
    bool editing = false,
         editingPlayerStartTile = false;
    int selectedTileIndex = 0;
};