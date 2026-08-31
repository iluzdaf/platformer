#include <format>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include "ui/tile_map_overlays.hpp"
#include "ui/imgui_manager.hpp"
#include "cameras/camera2d.hpp"
#include "tile_map/tile_map.hpp"

namespace
{
    TileGridOnScreen screenSpaceOf(
        const ImGuiManager &imGuiManager,
        const Camera2D &camera,
        const TileMap &tileMap)
    {
        return tileGridOnScreen(
            camera.getTopLeftPosition(),
            camera.getZoom(),
            imGuiManager.getUiScale(),
            tileMap.getTileSize());
    }
}

TileGridOnScreen tileGridOnScreen(
    glm::vec2 cameraTopLeft,
    float zoom,
    glm::vec2 uiScale,
    int tileSize)
{
    float tileSizeInWorld = static_cast<float>(tileSize);
    glm::vec2 snappedTopLeft = glm::floor(cameraTopLeft / tileSizeInWorld) * tileSizeInWorld;

    return {
        ((snappedTopLeft - cameraTopLeft) * zoom) / uiScale,
        (glm::vec2(tileSizeInWorld) * zoom) / uiScale,
        glm::floor(cameraTopLeft / tileSizeInWorld)};
}

void drawTileGrid(const ImGuiManager &imGuiManager, const Camera2D &camera, const TileMap &tileMap)
{
    TileGridOnScreen grid = screenSpaceOf(imGuiManager, camera, tileMap);
    ImVec2 uiDimensions = imGuiManager.getUiDimensions();
    ImDrawList *drawList = ImGui::GetBackgroundDrawList();

    for (float x = grid.firstLine.x; x < uiDimensions.x; x += grid.spacing.x)
        drawList->AddLine(ImVec2(x, 0), ImVec2(x, uiDimensions.y), IM_COL32(100, 100, 100, 255));

    for (float y = grid.firstLine.y; y < uiDimensions.y; y += grid.spacing.y)
        drawList->AddLine(ImVec2(0, y), ImVec2(uiDimensions.x, y), IM_COL32(100, 100, 100, 255));
}

void drawTileInfo(const ImGuiManager &imGuiManager, const Camera2D &camera, const TileMap &tileMap)
{
    TileGridOnScreen grid = screenSpaceOf(imGuiManager, camera, tileMap);
    ImDrawList *drawList = ImGui::GetBackgroundDrawList();

    for (float y = grid.firstLine.y, tileY = static_cast<float>(grid.topLeftTilePosition.y);
         tileY < tileMap.getHeight();
         y += grid.spacing.y, ++tileY)
    {
        if (tileY < 0)
            continue;

        for (float x = grid.firstLine.x, tileX = static_cast<float>(grid.topLeftTilePosition.x);
             tileX < tileMap.getWidth();
             x += grid.spacing.x, ++tileX)
        {
            if (tileX < 0)
                continue;

            int tileIndex = tileMap.tilePositionToTileIndex({tileX, tileY});
            std::string label = std::format("{},{}\n{}", tileX, tileY, tileIndex);
            drawList->AddText(ImVec2(x + 2, y + 2), IM_COL32(255, 255, 255, 200), label.c_str());
        }
    }
}
