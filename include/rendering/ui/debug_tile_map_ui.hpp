#pragma once
#include <glm/gtc/matrix_transform.hpp>
class ImGuiManager;
class Camera2D;
class TileMap;
class ImDrawList;
class ImVec2;

class DebugTileMapUi
{
public:
    void draw(
        const ImGuiManager &imGuiManager,
        const Camera2D &camera,
        const TileMap &tileMap,
        bool shouldDrawGrid,
        bool shouldDrawTileInfo);

private:
    void drawGridLines(
        ImDrawList *drawList,
        float tileSizeImgui,
        const ImVec2 &screenOffset,
        const ImVec2 &uiDimensions);
    void drawTileInfo(
        ImDrawList *drawList,
        const ImGuiManager &imGuiManager,
        const Camera2D &camera,
        const TileMap &tileMap,
        const ImVec2 &screenOffset,
        float tileSizeImgui,
        glm::ivec2 topLeftTilePosition);
};