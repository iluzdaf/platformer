#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include "rendering/debug_renderer.hpp"
#include "rendering/debug_renderer_data.hpp"
#include "rendering/texture2d.hpp"
#include "cameras/camera2d.hpp"
#include "game/tile_map/tile_map.hpp"
#include "game/player/player.hpp"

DebugRenderer::DebugRenderer(
    GLFWwindow *window,
    float screenWidth,
    float screenHeight,
    const DebugRendererData &data)
    : screenWidth(screenWidth),
      screenHeight(screenHeight),
      shouldDrawGrid(data.shouldDrawGrid),
      shouldDrawPlayerAABBs(data.shouldDrawPlayerAABBs),
      shouldDrawTileMapAABBs(data.shouldDrawTileMapAABBs),
      showDebugControls(data.showDebugControls)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 150");
}

DebugRenderer::~DebugRenderer()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void DebugRenderer::drawGrid(
    ImDrawList *drawList,
    const TileMap &tileMap,
    const Camera2D &camera)
{
    glm::vec2 cameraTopLeft = camera.getTopLeftPosition();
    float tileSize = tileMap.getTileSize();
    float worldOffsetX = fmod(cameraTopLeft.x, tileSize);
    float worldOffsetY = fmod(cameraTopLeft.y, tileSize);
    if (worldOffsetX < 0.0f)
        worldOffsetX += tileSize;
    if (worldOffsetY < 0.0f)
        worldOffsetY += tileSize;

    glm::vec2 offsetWorld = glm::vec2(-worldOffsetX, -worldOffsetY);
    ImVec2 offset = worldToScreen(offsetWorld + cameraTopLeft, camera);
    float tileSizeImgui = tileSize * camera.getZoom() / uiScale.x;

    for (float x = offset.x; x < uiWidth; x += tileSizeImgui)
    {
        drawList->AddLine(ImVec2(x, 0), ImVec2(x, uiHeight), IM_COL32(100, 100, 100, 255));
    }

    for (float y = offset.y; y < uiHeight; y += tileSizeImgui)
    {
        drawList->AddLine(ImVec2(0, y), ImVec2(uiWidth, y), IM_COL32(100, 100, 100, 255));
    }
}

void DebugRenderer::drawPlayerAABBs(
    ImDrawList *drawList,
    const Player &player,
    const Camera2D &camera)
{
    drawAABB(drawList, player.getAABB(), camera, IM_COL32(0, 255, 0, 255));
    PlayerState playerState = player.getPlayerState();
    addDebugAABB(playerState.collisionAABBX, IM_COL32(255, 255, 0, 255), 0.1f);
    addDebugAABB(playerState.collisionAABBY, IM_COL32(255, 255, 0, 255), 0.1f);
}

void DebugRenderer::drawTileMapAABBs(
    ImDrawList *drawList,
    const TileMap &tileMap,
    const Camera2D &camera)
{
    auto tilePositions = tileMap.getTilePositionsAt(camera.getTopLeftPosition(), glm::vec2(screenWidth, screenHeight));
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
            tile.getAABBAt(tileWorldPosition),
            camera,
            tile.isSpikes() ? IM_COL32(255, 0, 0, 255) : IM_COL32(0, 255, 0, 255));
    }
}

void DebugRenderer::drawAABB(
    ImDrawList *drawList,
    AABB aabb,
    const Camera2D &camera,
    ImU32 color)
{
    if (aabb.isEmpty())
    {
        return;
    }
    ImVec2 topLeft = worldToScreen(aabb.position, camera);
    ImVec2 bottomRight = worldToScreen(aabb.position + aabb.size, camera);
    drawList->AddRect(topLeft, bottomRight, color);
}

void DebugRenderer::update(
    float deltaTime,
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

    ImGuiIO &io = ImGui::GetIO();
    uiWidth = io.DisplaySize.x;
    uiHeight = io.DisplaySize.y;
    uiScale = glm::vec2(screenWidth, screenHeight) / glm::vec2(uiWidth, uiHeight);

    ImVec2 mouseScreenPosition = ImGui::GetMousePos();
    glm::vec2 worldPosition = screenToWorld(mouseScreenPosition, camera);
    glm::ivec2 tilePosition = tileMap.getTilePositionAt(worldPosition);
    if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
        tileMap.setTileIndex(tilePosition, selectedTileIndex);
    }
}

void DebugRenderer::draw(
    const Camera2D &camera,
    const TileMap &tileMap,
    const Player &player,
    const Texture2D &tileSet)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImDrawList *drawList = ImGui::GetBackgroundDrawList();

    if (shouldDrawGrid)
    {
        drawGrid(drawList, tileMap, camera);
    }

    if (shouldDrawPlayerAABBs)
    {
        drawPlayerAABBs(drawList, player, camera);
    }

    if (shouldDrawTileMapAABBs)
    {
        drawTileMapAABBs(drawList, tileMap, camera);
    }

    drawDebugAABBs(drawList, camera);

    if (showDebugControls)
    {
        ImGui::SetNextWindowSize(ImVec2(150, 100));
        ImGui::Begin("Debug");
        if (ImGui::Button("Step"))
            onStep();
        ImGui::SameLine();
        if (ImGui::Button("Play"))
            onPlay();
        ImGui::End();
    }

    drawTileMapControls(tileMap, tileSet);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void DebugRenderer::resize(float screenWidth, float screenHeight)
{
    this->screenWidth = screenWidth;
    this->screenHeight = screenHeight;
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

ImVec2 DebugRenderer::worldToScreen(glm::vec2 worldPosition, const Camera2D &camera) const
{
    glm::vec2 screenPosition = ((worldPosition - camera.getTopLeftPosition()) * camera.getZoom()) / uiScale;
    return ImVec2(screenPosition.x, screenPosition.y);
}

glm::vec2 DebugRenderer::screenToWorld(ImVec2 screenPosition, const Camera2D &camera) const
{
    glm::vec2 screen(screenPosition.x, screenPosition.y);
    glm::vec2 worldPos = (screen * uiScale) / camera.getZoom() + camera.getTopLeftPosition();
    return worldPos;
}

void DebugRenderer::drawDebugAABBs(ImDrawList *drawList, const Camera2D &camera)
{
    for (const auto &[hash, debugAABB] : debugAABBs)
    {
        drawAABB(drawList, debugAABB.box, camera, debugAABB.color);
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
        ImVec2 uv0(tileSetX * uvSize, tileSetY * uvSize);
        ImVec2 uv1((tileSetX + 1) * uvSize, (tileSetY + 1) * uvSize);

        int previouslySelectedTileIndex = selectedTileIndex;

        if (selectedTileIndex == tileIndex)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(255, 255, 0, 255));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(255, 255, 0, 255));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(255, 255, 0, 255));
        }

        if (ImGui::ImageButton("##tile", imguiTextureID, ImVec2(32, 32), uv0, uv1))
        {
            selectedTileIndex = tileIndex;
        }

        if (previouslySelectedTileIndex == tileIndex)
        {
            ImGui::PopStyleColor(3);
        }

        if (++count % columns != 0)
            ImGui::SameLine();

        ImGui::PopID();
    }

    if (ImGui::Button("Save"))
        tileMap.save();

    ImGui::End();
}