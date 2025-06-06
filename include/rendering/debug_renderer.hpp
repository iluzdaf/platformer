#pragma once
#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <unordered_map>
#include "physics/aabb.hpp"
class GLFWwindow;
class Camera2D;
class TileMap;
class Player;

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
        float screenHeight);
    ~DebugRenderer();
    void drawAABB(
        ImDrawList *drawList,
        const AABB &aabb,
        const glm::vec2 &cameraTopLeft,
        float zoom,
        const glm::vec2 &scale,
        ImU32 color);
    void draw(const Camera2D &camera,
              const TileMap &tileMap,
              const Player &player);
    void resize(float screenWidth, float screenHeight);
    void update(float deltaTime);

private:
    float screenWidth = 800, screenHeight = 600;
    std::unordered_map<std::size_t, DebugAABB> debugAABBs;

    void addDebugAABB(const AABB& aabb, ImU32 color, float duration);
};