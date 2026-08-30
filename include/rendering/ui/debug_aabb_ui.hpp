#pragma once
#include <imgui.h>
#include <unordered_map>
#include "physics/aabb.hpp"
class ImGuiManager;
class Camera2D;
class Player;
class TileMap;
class Level;

class DebugAABBUi
{
public:
    struct DebugAABB
    {
        AABB box;
        ImU32 color;
        float lifetime;
    };

    void drawOverlay(
        const ImGuiManager &imGuiManager,
        const Camera2D &camera,
        const Level &level,
        const Player &player,
        bool shouldDrawPlayerAABBs,
        bool shouldDrawTileMapAABBs);
    void update(float deltaTime);

private:
    std::unordered_map<std::size_t, DebugAABB> debugAABBs;

    void addDebugAABB(AABB aabb, ImU32 color, float duration);
    void drawDebugAABBs(
        ImDrawList *drawList,
        const ImGuiManager &imGuiManager,
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
        glm::ivec2 playerStartTile,
        const Camera2D &camera);
    void drawAABB(
        ImDrawList *drawList,
        const ImGuiManager &imGuiManager,
        AABB aabb,
        const Camera2D &camera,
        ImU32 color);
};