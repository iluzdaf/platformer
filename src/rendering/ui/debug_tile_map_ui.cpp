#include "rendering/ui/debug_tile_map_ui.hpp"
#include "cameras/camera2d.hpp"
#include "game/tile_map/tile_map.hpp"
#include "rendering/ui/imgui_manager.hpp"

void DebugTileMapUi::draw(
    ImGuiManager &imGuiManager,
    const Camera2D &camera,
    const TileMap &tileMap,
    bool shouldDrawGrid,
    bool shouldDrawTileInfo)
{
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

    glm::vec2 cameraTopLeft = camera.getTopLeftPosition();
    float tileSize = tileMap.getTileSize();
    glm::vec2 snappedTopLeft = glm::floor(cameraTopLeft / tileSize) * tileSize;
    ImVec2 screenOffset = imGuiManager.worldToScreen(snappedTopLeft, camera.getZoom(), camera.getTopLeftPosition());
    float tileSizeImgui = tileSize * camera.getZoom() / imGuiManager.getUiScale().x;

    ImDrawList *drawList = ImGui::GetBackgroundDrawList();

    if (shouldDrawGrid)
    {
        ImVec2 uiDimensions = imGuiManager.getUiDimensions();
        drawGridLines(drawList, tileSizeImgui, screenOffset, uiDimensions);
    }

    if (shouldDrawTileInfo)
    {
        glm::ivec2 topLeftTilePosition = glm::floor(cameraTopLeft / static_cast<float>(tileSize));
        drawTileInfo(
            drawList,
            imGuiManager,
            camera,
            tileMap,
            screenOffset,
            tileSizeImgui,
            topLeftTilePosition);
    }
}

void DebugTileMapUi::drawGridLines(
    ImDrawList *drawList,
    float tileSizeImgui,
    const ImVec2 &screenOffset,
    const ImVec2 &uiDimensions)
{
    for (float x = screenOffset.x; x < uiDimensions.x; x += tileSizeImgui)
        drawList->AddLine(ImVec2(x, 0), ImVec2(x, uiDimensions.y), IM_COL32(100, 100, 100, 255));

    for (float y = screenOffset.y; y < uiDimensions.y; y += tileSizeImgui)
        drawList->AddLine(ImVec2(0, y), ImVec2(uiDimensions.x, y), IM_COL32(100, 100, 100, 255));
}

void DebugTileMapUi::drawTileInfo(
    ImDrawList *drawList,
    const ImGuiManager &imGuiManager,
    const Camera2D &camera,
    const TileMap &tileMap,
    const ImVec2 &screenOffset,
    float tileSizeImgui,
    glm::ivec2 topLeftTilePosition)
{
    for (float y = screenOffset.y, tileY = topLeftTilePosition.y; tileY < tileMap.getHeight(); y += tileSizeImgui, ++tileY)
    {
        if (tileY < 0)
            continue;

        for (float x = screenOffset.x, tileX = topLeftTilePosition.x; tileX < tileMap.getWidth(); x += tileSizeImgui, ++tileX)
        {
            if (tileX < 0)
                continue;

            int tileIndex = tileMap.tilePositionToTileIndex({tileX, tileY});
            std::string label = std::format("{},{}\n{}", tileX, tileY, tileIndex);
            drawList->AddText(ImVec2(x + 2, y + 2), IM_COL32(255, 255, 255, 200), label.c_str());
        }
    }
}