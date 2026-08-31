#pragma once

#include <glm/gtc/matrix_transform.hpp>

class ImGuiManager;
class Camera2D;
class TileMap;

struct TileGridOnScreen
{
    glm::vec2 firstLine;
    glm::vec2 spacing;
    glm::ivec2 topLeftTilePosition;
};

TileGridOnScreen tileGridOnScreen(
    glm::vec2 cameraTopLeft,
    float zoom,
    glm::vec2 uiScale,
    int tileSize);

void drawTileGrid(const ImGuiManager &imGuiManager, const Camera2D &camera, const TileMap &tileMap);

void drawTileInfo(const ImGuiManager &imGuiManager, const Camera2D &camera, const TileMap &tileMap);
