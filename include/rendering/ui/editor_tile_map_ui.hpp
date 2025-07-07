#pragma once
#include <signals.hpp>
#include <string>

class ImGuiManager;
class TileMap;
class Texture2D;
class Camera2D;

class EditorTileMapUi
{
public:
    void draw(
        const ImGuiManager &imGuiManager,
        const TileMap &tileMap,
        const Texture2D &tileSet,
        bool shouldDrawTileMapEditor);
    void update(
        const ImGuiManager &imGuiManager,
        const Camera2D &camera,
        TileMap &tileMap);

    fteng::signal<void(const std::string &)> onLoadLevel;

private:
    bool editing = false,
         editingPlayerStartTile = false;
    int selectedTileIndex = 0;
};