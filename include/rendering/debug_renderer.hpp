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
    void drawGrid(
        ImDrawList *drawList,
        const TileMap &tileMap,
        glm::vec2 cameraPosition,
        glm::vec2 cameraTopLeft,
        float zoom,
        glm::vec2 scale,
        float displayWidth,
        float displayHeight);
    void drawPlayerAABBs(
        ImDrawList *drawList,
        const Player &player,
        glm::vec2 cameraTopLeft,
        float zoom,
        glm::vec2 scale);
    void drawTileMapAABBs(
        ImDrawList *drawList,
        const TileMap &tileMap,
        glm::vec2 cameraTopLeft,
        float zoom,
        glm::vec2 scale);
    void drawAABB(ImDrawList *drawList,
                  AABB aabb,
                  glm::vec2 cameraTopLeft,
                  float zoom,
                  glm::vec2 scale,
                  ImU32 color);
    void draw(const Camera2D &camera,
              const TileMap &tileMap,
              const Player &player);
    void resize(float screenWidth, float screenHeight);
    void update(float deltaTime);

    fteng::signal<void()> onPlay;
    fteng::signal<void()> onStep;

private:
    float screenWidth = 800, screenHeight = 600;
    std::unordered_map<std::size_t, DebugAABB> debugAABBs;
    bool shouldDrawGrid = false,
         shouldDrawPlayerAABBs = false,
         shouldDrawTileMapAABBs = false,
         showDebugControls = false;

    void addDebugAABB(AABB aabb, ImU32 color, float duration);
};