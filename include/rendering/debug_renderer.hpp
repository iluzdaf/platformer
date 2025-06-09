#pragma once
#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <unordered_map>
#include <signals.hpp>
#include "physics/aabb.hpp"
class GLFWwindow;
class Camera2D;
class TileMap;
class Player;
class DebugRendererData;
class Texture2D;
class ImGuiManager;

class DebugRenderer
{
public:
    struct DebugAABB
    {
        AABB box;
        ImU32 color;
        float lifetime;
    };

    DebugRenderer(
        float screenWidth,
        float screenHeight,
        const DebugRendererData &data);
    void draw(
        ImGuiManager &imGuiManager,
        const Camera2D &camera,
        const TileMap &tileMap,
        const Player &player,
        const Texture2D &tileSet);
    void resize(int windowWidth, int windowHeight);
    void update(
        float deltaTime,
        ImGuiManager &imGuiManager,
        const Camera2D &camera,
        TileMap &tileMap);

    fteng::signal<void()> onPlay;
    fteng::signal<void()> onStep;
    fteng::signal<void()> onRespawn;
    fteng::signal<void(const std::string &)> onLoadLevel;
    fteng::signal<void()> onToggleZoom;

private:
    float windowWidth = 800, windowHeight = 600;
    std::unordered_map<std::size_t, DebugAABB> debugAABBs;
    bool shouldDrawGrid = false,
         shouldDrawPlayerAABBs = false,
         shouldDrawTileMapAABBs = false,
         showDebugControls = false,
         editingPlayerStartTile = false,
         shouldDrawTileInfo = false,
         showTileMapControls = false;
    int selectedTileIndex = 0;

    void addDebugAABB(
        AABB aabb,
        ImU32 color,
        float duration);
    void drawDebugAABBs(
        ImDrawList *drawList,
        ImGuiManager &imGuiManager,
        const Camera2D &camera);
    void drawGrid(
        ImDrawList *drawList,
        const ImGuiManager &imGuiManager,
        const TileMap &tileMap,
        const Camera2D &camera);
    void drawPlayerAABBs(
        ImDrawList *drawList,
        const ImGuiManager &imGuiManager,
        const Player &player,
        const Camera2D &camera);
    void drawTileMapAABBs(
        ImDrawList *drawList,
        const ImGuiManager &imGuiManager,
        const TileMap &tileMap,
        const Camera2D &camera);
    void drawAABB(
        ImDrawList *drawList,
        const ImGuiManager& imGuiManager,
        AABB aabb,
        const Camera2D &camera,
        ImU32 color);
    void drawTileMapControls(
        const TileMap &tileMap,
        const Texture2D &tileSet);
    glm::vec2 calculateTileMapWorldOffset(
        glm::vec2 cameraTopLeft,
        float tileSize) const;
    void drawTileInfo(
        ImDrawList *drawList,
        const ImGuiManager& imGuiManager,
        const Camera2D &camera,
        const TileMap &tileMap);
    void drawDebugControls();
};