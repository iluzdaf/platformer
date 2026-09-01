#include <format>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include "ui/tile_map_overlays.hpp"
#include "ui/imgui_manager.hpp"
#include "cameras/camera2d.hpp"
#include "tile_map/tile_map.hpp"

namespace
{
    constexpr unsigned int GridColour = IM_COL32(100, 100, 100, 255);

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

    if (grid.spacing.x <= 0.0f || grid.spacing.y <= 0.0f)
        return;

    for (int column = 0;; ++column)
    {
        float x = lineAcross(grid, column);
        if (x >= uiDimensions.x)
            break;

        drawList->AddLine(ImVec2(x, 0), ImVec2(x, uiDimensions.y), GridColour);
    }

    for (int row = 0;; ++row)
    {
        float y = lineDown(grid, row);
        if (y >= uiDimensions.y)
            break;

        drawList->AddLine(ImVec2(0, y), ImVec2(uiDimensions.x, y), GridColour);
    }
}

void drawTileInfo(const ImGuiManager &imGuiManager, const Camera2D &camera, const TileMap &tileMap)
{
    TileGridOnScreen grid = screenSpaceOf(imGuiManager, camera, tileMap);
    ImDrawList *drawList = ImGui::GetBackgroundDrawList();

    for (int row = 0; grid.topLeftTilePosition.y + row < tileMap.getHeight(); ++row)
    {
        int tileY = grid.topLeftTilePosition.y + row;
        if (tileY < 0)
            continue;

        float y = lineDown(grid, row);

        for (int column = 0; grid.topLeftTilePosition.x + column < tileMap.getWidth(); ++column)
        {
            int tileX = grid.topLeftTilePosition.x + column;
            if (tileX < 0)
                continue;

            float x = lineAcross(grid, column);
            int tileIndex = tileMap.tilePositionToTileIndex({tileX, tileY});
            std::string label = std::format("{},{}\n{}", tileX, tileY, tileIndex);
            drawList->AddText(ImVec2(x + 2, y + 2), IM_COL32(255, 255, 255, 200), label.c_str());
        }
    }
}
