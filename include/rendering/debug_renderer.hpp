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
        GLFWwindow *window,
        float screenWidth,
        float screenHeight,
        const DebugRendererData &data);
    ~DebugRenderer();
    void draw(const Camera2D &camera,
              const TileMap &tileMap,
              const Player &player,
              const Texture2D &tileSet);
    void resize(float screenWidth, float screenHeight);
    void update(
        float deltaTime,
        const Camera2D &camera,
        TileMap &tileMap);

    fteng::signal<void()> onPlay;
    fteng::signal<void()> onStep;
    fteng::signal<void()> onRespawn;
    fteng::signal<void(const std::string&)> onLoadLevel;

private:
    float screenWidth = 800, screenHeight = 600,
          uiWidth = 800, uiHeight = 600;
    glm::vec2 uiScale = glm::vec2(1.0f);
    std::unordered_map<std::size_t, DebugAABB> debugAABBs;
    bool shouldDrawGrid = false,
         shouldDrawPlayerAABBs = false,
         shouldDrawTileMapAABBs = false,
         showDebugControls = false,
         editingPlayerStartTile = false;
    int selectedTileIndex = 0;

    void addDebugAABB(
        AABB aabb, 
        ImU32 color, 
        float duration);
    ImVec2 worldToScreen(
        glm::vec2 worldPosition, 
        const Camera2D &camera) const;
    glm::vec2 screenToWorld(
        ImVec2 screenPosition, 
        const Camera2D &camera) const;
    void drawDebugAABBs(
        ImDrawList *drawList, 
        const Camera2D &camera);
    void drawGrid(
        ImDrawList *drawList,
        const TileMap &tileMap,
        const Camera2D &camera);
    void drawPlayerAABBs(
        ImDrawList *drawList,
        const Player &player,
        const Camera2D &camera);
    void drawTileMapAABBs(
        ImDrawList *drawList,
        const TileMap &tileMap,
        const Camera2D &camera);
    void drawAABB(
        ImDrawList *drawList,
        AABB aabb,
        const Camera2D &camera,
        ImU32 color);
    void drawTileMapControls(
        const TileMap &tileMap, 
        const Texture2D &tileSet);
};