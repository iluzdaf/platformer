#pragma once
#include <signals.hpp>
class ImGuiManager;
class TileMap;
class Texture2D;
class Camera2D;

class EditorTileMapUi
{
public:
    void draw(
        ImGuiManager &imGuiManager,
        const TileMap &tileMap,
        const Texture2D &tileSet,
        bool shouldDrawTileMapEditor);
    void update(
        ImGuiManager &imGuiManager,
        const Camera2D &camera,
        TileMap &tileMap);

    fteng::signal<void(const std::string &)> onLoadLevel;

private:
    bool editing = false;
    bool editingPlayerStartTile = false;
    int selectedTileIndex = 0;
};