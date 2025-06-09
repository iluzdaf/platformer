#include "rendering/debug_renderer.hpp"
#include "rendering/debug_renderer_data.hpp"
#include "rendering/texture2d.hpp"
#include "rendering/ui/imgui_manager.hpp"
#include "cameras/camera2d.hpp"
#include "game/tile_map/tile_map.hpp"
#include "game/player/player.hpp"

DebugRenderer::DebugRenderer(
    float windowWidth,
    float windowHeight,
    const DebugRendererData &data)
    : windowWidth(windowWidth),
      windowHeight(windowHeight),
      shouldDrawGrid(data.shouldDrawGrid),
      shouldDrawPlayerAABBs(data.shouldDrawPlayerAABBs),
      shouldDrawTileMapAABBs(data.shouldDrawTileMapAABBs),
      showDebugControls(data.showDebugControls),
      shouldDrawTileInfo(data.shouldDrawTileInfo),
      showTileMapControls(data.showTileMapControls)
{
}

void DebugRenderer::drawGrid(
    ImDrawList *drawList,
    const ImGuiManager &imGuiManager,
    const TileMap &tileMap,
    const Camera2D &camera)
{
    glm::vec2 cameraTopLeft = camera.getTopLeftPosition();
    float tileSize = tileMap.getTileSize();
    glm::vec2 tileMapWorldOffset = calculateTileMapWorldOffset(cameraTopLeft, tileSize);
    ImVec2 offset = imGuiManager.worldToScreen(tileMapWorldOffset + cameraTopLeft, camera);
    float tileSizeImgui = tileSize * camera.getZoom() / imGuiManager.getUiScale().x;

    for (float screenX = offset.x; screenX < imGuiManager.getUiDimensions().x; screenX += tileSizeImgui)
    {
        drawList->AddLine(ImVec2(screenX, 0), ImVec2(screenX, imGuiManager.getUiDimensions().y), IM_COL32(100, 100, 100, 255));
    }

    for (float screenY = offset.y; screenY < imGuiManager.getUiDimensions().y; screenY += tileSizeImgui)
    {
        drawList->AddLine(ImVec2(0, screenY), ImVec2(imGuiManager.getUiDimensions().x, screenY), IM_COL32(100, 100, 100, 255));
    }
}

void DebugRenderer::drawPlayerAABBs(
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

void DebugRenderer::drawTileMapAABBs(
    ImDrawList *drawList,
    const ImGuiManager &imGuiManager,
    const TileMap &tileMap,
    const Camera2D &camera)
{
    auto tilePositions = tileMap.getTilePositionsAt(camera.getTopLeftPosition(), glm::vec2(windowWidth, windowHeight));
    for (auto tilePosition : tilePositions)
    {
        auto tile = tileMap.getTile(tilePosition);
        if (!tile.isSpikes() && !tile.isPickup())
        {
            continue;
        }

        glm::vec2 tileWorldPosition = tileMap.getTileWorldPosition(tilePosition);
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

void DebugRenderer::drawAABB(
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
    ImVec2 topLeft = imGuiManager.worldToScreen(aabb.position, camera);
    ImVec2 bottomRight = imGuiManager.worldToScreen(aabb.position + aabb.size, camera);
    drawList->AddRect(topLeft, bottomRight, color);
}

void DebugRenderer::update(
    float deltaTime,
    ImGuiManager &imGuiManager,
    const Camera2D &camera,
    TileMap &tileMap)
{
    for (auto it = debugAABBs.begin(); it != debugAABBs.end();)
    {
        it->second.lifetime -= deltaTime;
        if (it->second.lifetime <= 0.0f)
            it = debugAABBs.erase(it);
        else
            ++it;
    }

    ImVec2 mouseScreenPosition = ImGui::GetMousePos();
    glm::vec2 worldPosition = imGuiManager.screenToWorld(mouseScreenPosition, camera);
    glm::ivec2 tilePosition = tileMap.getTilePositionAt(worldPosition);
    if (!tileMap.validTilePosition(tilePosition))
    {
        return;
    }

    if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && !imGuiManager.getIO().WantCaptureMouse)
    {
        if (editingPlayerStartTile)
        {
            tileMap.setPlayerStartTile(tilePosition);
            editingPlayerStartTile = false;
        }
        else
        {
            tileMap.setTileIndex(tilePosition, selectedTileIndex);
        }
    }
}

void DebugRenderer::draw(
    ImGuiManager &imGuiManager,
    const Camera2D &camera,
    const TileMap &tileMap,
    const Player &player,
    const Texture2D &tileSet)
{
    imGuiManager.newFrame();

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImDrawList *drawList = ImGui::GetBackgroundDrawList();

    if (shouldDrawGrid)
    {
        drawGrid(drawList, imGuiManager, tileMap, camera);
    }

    if (shouldDrawTileInfo)
    {
        drawTileInfo(drawList, imGuiManager, camera, tileMap);
    }

    if (shouldDrawPlayerAABBs)
    {
        drawPlayerAABBs(drawList, imGuiManager, player, camera);
    }

    if (shouldDrawTileMapAABBs)
    {
        drawTileMapAABBs(drawList, imGuiManager, tileMap, camera);
    }

    drawDebugAABBs(drawList, imGuiManager, camera);

    if (showDebugControls)
    {
        drawDebugControls();
    }

    if (showTileMapControls)
    {
        drawTileMapControls(tileMap, tileSet);
    }

    imGuiManager.render();
}

void DebugRenderer::resize(int windowWidth, int windowHeight)
{
    this->windowWidth = windowWidth;
    this->windowHeight = windowHeight;
}

void DebugRenderer::addDebugAABB(AABB aabb, ImU32 color, float duration)
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

void DebugRenderer::drawDebugAABBs(
    ImDrawList *drawList,
    ImGuiManager &imGuiManager,
    const Camera2D &camera)
{
    for (const auto &[hash, debugAABB] : debugAABBs)
    {
        drawAABB(drawList, imGuiManager, debugAABB.box, camera, debugAABB.color);
    }
}

void DebugRenderer::drawTileMapControls(
    const TileMap &tileMap,
    const Texture2D &tileSet)
{
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 200, 0));
    ImGui::SetNextWindowSize(ImVec2(200, 400));
    ImGui::Begin("TileMap");
    const auto &tiles = tileMap.getTiles();
    int columns = 4;
    int count = 0;
    int tileSize = tileMap.getTileSize();
    int tileSetWidth = tileSet.getWidth();
    int tilesPerRow = tileSetWidth / tileSize;
    float uvSize = static_cast<float>(tileSize) / static_cast<float>(tileSetWidth);
    ImTextureID imguiTextureID = (ImTextureID)(intptr_t)tileSet.getTextureID();

    for (const auto &[tileIndex, tile] : tiles)
    {
        ImGui::PushID(tileIndex);

        int tileSetX = tileIndex % tilesPerRow;
        int tileSetY = tileIndex / tilesPerRow;
        ImVec2 uv0(tileSetX * uvSize, (tileSetY + 1) * uvSize);
        ImVec2 uv1((tileSetX + 1) * uvSize, tileSetY * uvSize);

        int previouslySelectedTileIndex = selectedTileIndex;

        if (previouslySelectedTileIndex == tileIndex)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(255, 255, 0, 255));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(255, 255, 0, 255));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(255, 255, 0, 255));
        }

        ImVec2 tilePos = ImGui::GetCursorScreenPos();
        if (ImGui::ImageButton("##tile", imguiTextureID, ImVec2(32, 32), uv0, uv1))
        {
            selectedTileIndex = tileIndex;
        }
        ImGui::GetWindowDrawList()->AddText(
            tilePos,
            IM_COL32(255, 255, 255, 255),
            std::to_string(tileIndex).c_str());

        if (previouslySelectedTileIndex == tileIndex)
        {
            ImGui::PopStyleColor(3);
        }

        if (++count % columns != 0)
            ImGui::SameLine();

        ImGui::PopID();
    }

    bool previouslyEditingPlayerStartTile = editingPlayerStartTile;
    if (previouslyEditingPlayerStartTile)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(255, 100, 255, 255));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(255, 100, 255, 255));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(255, 100, 255, 255));
    }

    if (ImGui::Button("Spawn", ImVec2(40, 38)))
    {
        editingPlayerStartTile = true;
        selectedTileIndex = 0;
    }

    if (previouslyEditingPlayerStartTile)
        ImGui::PopStyleColor(3);

    if (ImGui::Button("Save", ImVec2(40, 38)))
        tileMap.save();

    ImGui::SameLine();
    if (ImGui::Button("Load", ImVec2(40, 38)))
        onLoadLevel("../assets/tile_maps/new_level.json");

    ImGui::End();
}

glm::vec2 DebugRenderer::calculateTileMapWorldOffset(glm::vec2 cameraTopLeft, float tileSize) const
{
    float worldOffsetX = fmod(cameraTopLeft.x, tileSize);
    float worldOffsetY = fmod(cameraTopLeft.y, tileSize);
    if (worldOffsetX < 0.0f)
        worldOffsetX += tileSize;
    if (worldOffsetY < 0.0f)
        worldOffsetY += tileSize;
    return glm::vec2(-worldOffsetX, -worldOffsetY);
}

void DebugRenderer::drawTileInfo(
    ImDrawList *drawList,
    const ImGuiManager &imGuiManager,
    const Camera2D &camera,
    const TileMap &tileMap)
{
    glm::vec2 cameraTopLeft = camera.getTopLeftPosition();
    int tileSize = tileMap.getTileSize();
    glm::vec2 tileMapWorldOffset = calculateTileMapWorldOffset(cameraTopLeft, tileSize);
    ImVec2 offset = imGuiManager.worldToScreen(tileMapWorldOffset + cameraTopLeft, camera);
    float tileSizeImgui = tileSize * camera.getZoom() / imGuiManager.getUiScale().x;
    glm::ivec2 topLeftTilePosition = glm::floor(cameraTopLeft / static_cast<float>(tileSize));
    for (float screenY = offset.y, tileY = topLeftTilePosition.y; tileY < tileMap.getHeight(); screenY += tileSizeImgui, ++tileY)
    {
        if (tileY < 0)
            continue;

        for (float screenX = offset.x, tileX = topLeftTilePosition.x; tileX < tileMap.getWidth(); screenX += tileSizeImgui, ++tileX)
        {
            if (tileX < 0)
                continue;

            int tileIndex = tileMap.getTileIndex(glm::ivec2(tileX, tileY));
            std::string label = std::format("{},{}\n{}", static_cast<int>(tileX), static_cast<int>(tileY), tileIndex);
            drawList->AddText(ImVec2(screenX + 2, screenY + 2), IM_COL32(255, 255, 255, 200), label.c_str());
        }
    }
}

void DebugRenderer::drawDebugControls()
{
    ImGui::SetNextWindowSize(ImVec2(200, 120));
    ImGui::Begin("Debug");
    if (ImGui::Button("Step"))
        onStep();
    ImGui::SameLine();
    if (ImGui::Button("Play"))
        onPlay();
    ImGui::SameLine();
    if (ImGui::Button("Respawn"))
        onRespawn();
    ImGui::SameLine();
    if (ImGui::Button("Zoom"))
        onToggleZoom();
    if (ImGui::Button("Tile Info"))
        shouldDrawTileInfo = !shouldDrawTileInfo;
    ImGui::SameLine();
    if (ImGui::Button("Grid"))
        shouldDrawGrid = !shouldDrawGrid;
    ImGui::SameLine();
    if (ImGui::Button("Player"))
        shouldDrawPlayerAABBs = !shouldDrawPlayerAABBs;
    if (ImGui::Button("TileMap"))
        shouldDrawTileMapAABBs = !shouldDrawTileMapAABBs;
    ImGui::End();
}