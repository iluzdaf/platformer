#include "rendering/ui/debug_aabb_ui.hpp"
#include "game/player/player.hpp"
#include "game/tile_map/tile_map.hpp"
#include "cameras/camera2d.hpp"
#include "rendering/ui/imgui_manager.hpp"

void DebugAABBUi::draw(
    ImGuiManager &imGuiManager,
    const Player &player,
    const TileMap &tileMap,
    const Camera2D &camera,
    bool shouldDrawPlayerAABBs,
    bool shouldDrawTileMapAABBs)
{
    ImDrawList *drawList = ImGui::GetBackgroundDrawList();

    if (shouldDrawPlayerAABBs)
    {
        drawPlayerAABBs(drawList, imGuiManager, player, camera);
    }

    if (shouldDrawTileMapAABBs)
    {
        drawTileMapAABBs(drawList, imGuiManager, tileMap, camera);
    }

    drawDebugAABBs(drawList, imGuiManager, camera);
}

void DebugAABBUi::update(float deltaTime)
{
    for (auto it = debugAABBs.begin(); it != debugAABBs.end();)
    {
        it->second.lifetime -= deltaTime;
        if (it->second.lifetime <= 0.0f)
            it = debugAABBs.erase(it);
        else
            ++it;
    }
}

void DebugAABBUi::drawPlayerAABBs(
    ImDrawList *drawList,
    const ImGuiManager &imGuiManager,
    const Player &player,
    const Camera2D &camera)
{
    drawAABB(drawList, imGuiManager, player.getAABB(), camera, IM_COL32(0, 255, 0, 255));
    PlayerState playerState = player.getPlayerState();
    addDebugAABB(playerState.collisionAABBX, IM_COL32(255, 255, 0, 255), 0.1f);
    addDebugAABB(playerState.collisionAABBY, IM_COL32(255, 255, 0, 255), 0.1f);
}

void DebugAABBUi::drawTileMapAABBs(
    ImDrawList *drawList,
    const ImGuiManager &imGuiManager,
    const TileMap &tileMap,
    const Camera2D &camera)
{
    auto tilePositions = tileMap.worldToTilePositions(camera.getTopLeftPosition(), camera.getWindowSize());
    for (auto tilePosition : tilePositions)
    {
        auto tile = tileMap.getTileAtTilePosition(tilePosition);
        if (!tile.isSpikes() && !tile.isPickup())
        {
            continue;
        }

        glm::vec2 tileWorldPosition = tileMap.tileToWorldPosition(tilePosition);
        drawAABB(
            drawList,
            imGuiManager,
            tile.getAABBAt(tileWorldPosition),
            camera,
            tile.isSpikes() ? IM_COL32(255, 0, 0, 255) : IM_COL32(0, 255, 0, 255));
    }

    drawAABB(
        drawList,
        imGuiManager,
        AABB(glm::vec2(0), glm::vec2(tileMap.getWorldWidth(), tileMap.getWorldHeight())),
        camera,
        IM_COL32(255, 255, 0, 255));

    glm::vec2 playerStartWorldPosition = tileMap.getPlayerStartWorldPosition();
    drawAABB(
        drawList,
        imGuiManager,
        AABB(playerStartWorldPosition, glm::vec2(tileMap.getTileSize())),
        camera,
        IM_COL32(255, 0, 255, 255));
}

void DebugAABBUi::drawAABB(
    ImDrawList *drawList,
    const ImGuiManager &imGuiManager,
    AABB aabb,
    const Camera2D &camera,
    ImU32 color)
{
    if (aabb.isEmpty())
    {
        return;
    }
    ImVec2 topLeft = imGuiManager.worldToScreen(aabb.position, camera.getZoom(), camera.getTopLeftPosition());
    ImVec2 bottomRight = imGuiManager.worldToScreen(aabb.position + aabb.size, camera.getZoom(), camera.getTopLeftPosition());
    drawList->AddRect(topLeft, bottomRight, color);
}

void DebugAABBUi::addDebugAABB(AABB aabb, ImU32 color, float duration)
{
    if (aabb.isEmpty())
    {
        return;
    }
    std::size_t hash = aabb.hash();
    auto it = debugAABBs.find(hash);
    if (it != debugAABBs.end())
    {
        it->second.lifetime = duration;
    }
    else
    {
        debugAABBs[hash] = DebugAABB{aabb, color, duration};
    }
}

void DebugAABBUi::drawDebugAABBs(
    ImDrawList *drawList,
    ImGuiManager &imGuiManager,
    const Camera2D &camera)
{
    for (const auto &[hash, debugAABB] : debugAABBs)
    {
        drawAABB(drawList, imGuiManager, debugAABB.box, camera, debugAABB.color);
    }
}