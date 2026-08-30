#include <format>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include "ui/tile_map_overlays.hpp"
#include "ui/imgui_manager.hpp"
#include "cameras/camera2d.hpp"
#include "tile_map/tile_map.hpp"

namespace
{
    struct TileScreenSpace
    {
        ImVec2 offset;
        float tileSize;
        glm::ivec2 topLeftTilePosition;
    };

    TileScreenSpace screenSpaceOf(
        const ImGuiManager &imGuiManager,
        const Camera2D &camera,
        const TileMap &tileMap)
    {
        glm::vec2 cameraTopLeft = camera.getTopLeftPosition();
        float tileSize = static_cast<float>(tileMap.getTileSize());
        glm::vec2 snappedTopLeft = glm::floor(cameraTopLeft / tileSize) * tileSize;

        return {
            imGuiManager.worldToScreen(snappedTopLeft, camera.getZoom(), cameraTopLeft),
            tileSize * camera.getZoom() / imGuiManager.getUiScale().x,
            glm::floor(cameraTopLeft / tileSize)};
    }
}

void drawTileGrid(const ImGuiManager &imGuiManager, const Camera2D &camera, const TileMap &tileMap)
{
    TileScreenSpace space = screenSpaceOf(imGuiManager, camera, tileMap);
    ImVec2 uiDimensions = imGuiManager.getUiDimensions();
    ImDrawList *drawList = ImGui::GetBackgroundDrawList();

    for (float x = space.offset.x; x < uiDimensions.x; x += space.tileSize)
        drawList->AddLine(ImVec2(x, 0), ImVec2(x, uiDimensions.y), IM_COL32(100, 100, 100, 255));

    for (float y = space.offset.y; y < uiDimensions.y; y += space.tileSize)
        drawList->AddLine(ImVec2(0, y), ImVec2(uiDimensions.x, y), IM_COL32(100, 100, 100, 255));
}

void drawTileInfo(const ImGuiManager &imGuiManager, const Camera2D &camera, const TileMap &tileMap)
{
    TileScreenSpace space = screenSpaceOf(imGuiManager, camera, tileMap);
    ImDrawList *drawList = ImGui::GetBackgroundDrawList();

    for (float y = space.offset.y, tileY = static_cast<float>(space.topLeftTilePosition.y);
         tileY < tileMap.getHeight();
         y += space.tileSize, ++tileY)
    {
        if (tileY < 0)
            continue;

        for (float x = space.offset.x, tileX = static_cast<float>(space.topLeftTilePosition.x);
             tileX < tileMap.getWidth();
             x += space.tileSize, ++tileX)
        {
            if (tileX < 0)
                continue;

            int tileIndex = tileMap.tilePositionToTileIndex({tileX, tileY});
            std::string label = std::format("{},{}\n{}", tileX, tileY, tileIndex);
            drawList->AddText(ImVec2(x + 2, y + 2), IM_COL32(255, 255, 255, 200), label.c_str());
        }
    }
}
