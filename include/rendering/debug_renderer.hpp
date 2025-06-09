#pragma once
#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <signals.hpp>
class Camera2D;
class TileMap;
class Player;
class DebugRendererData;
class Texture2D;
class ImGuiManager;

class DebugRenderer
{
public:

    explicit DebugRenderer(const DebugRendererData &data);
    void draw(
        ImGuiManager &imGuiManager,
        const TileMap &tileMap,
        const Texture2D &tileSet);
    void update(
        ImGuiManager &imGuiManager,
        const Camera2D &camera,
        TileMap &tileMap);

    fteng::signal<void()> onPlay,
        onStep,
        onRespawn,
        onToggleZoom,
        onToggleDrawGrid,
        onToggleDrawTileInfo,
        onToggleDrawPlayerAABBs,
        onToggleDrawTileMapAABBs;
    fteng::signal<void(const std::string &)> onLoadLevel;

private:
    bool showDebugControls = false,
         editingPlayerStartTile = false,
         showTileMapControls = false;
    int selectedTileIndex = 0;

    void drawTileMapControls(
        const TileMap &tileMap,
        const Texture2D &tileSet);
    void drawDebugControls();
};