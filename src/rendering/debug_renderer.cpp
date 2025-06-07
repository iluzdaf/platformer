#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include "rendering/debug_renderer.hpp"
#include "rendering/debug_renderer_data.hpp"
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
    glm::vec2 cameraPosition,
    glm::vec2 cameraTopLeft,
    float zoom,
    glm::vec2 scale,
    float displayWidth,
    float displayHeight)
{
    float scaledTileInFramebuffer = tileMap.getTileSize() * zoom;
    float offsetX_fb = fmod(cameraPosition.x * zoom, scaledTileInFramebuffer);
    float offsetY_fb = fmod(cameraPosition.y * zoom, scaledTileInFramebuffer);
    float rawOffsetX = fmod(cameraTopLeft.x * zoom, scaledTileInFramebuffer);
    if (rawOffsetX < 0.0f)
        rawOffsetX += scaledTileInFramebuffer;
    float offsetX = -rawOffsetX / scale.x;
    float rawOffsetY = fmod(cameraTopLeft.y * zoom, scaledTileInFramebuffer);
    if (rawOffsetY < 0.0f)
        rawOffsetY += scaledTileInFramebuffer;
    float offsetY = -rawOffsetY / scale.y;
    float tileSizeImgui = scaledTileInFramebuffer / scale.x;

    for (float x = offsetX; x < displayWidth; x += tileSizeImgui)
    {
        drawList->AddLine(ImVec2(x, 0), ImVec2(x, displayHeight), IM_COL32(100, 100, 100, 255));
    }

    for (float y = offsetY; y < displayHeight; y += tileSizeImgui)
    {
        drawList->AddLine(ImVec2(0, y), ImVec2(displayWidth, y), IM_COL32(100, 100, 100, 255));
    }
}

void DebugRenderer::drawPlayerAABBs(
    ImDrawList *drawList,
    const Player &player,
    glm::vec2 cameraTopLeft,
    float zoom,
    glm::vec2 scale)
{
    drawAABB(drawList, player.getAABB(), cameraTopLeft, zoom, scale, IM_COL32(0, 255, 0, 255));
    PlayerState playerState = player.getPlayerState();
    addDebugAABB(playerState.collisionAABBX, IM_COL32(255, 255, 0, 255), 0.1f);
    addDebugAABB(playerState.collisionAABBY, IM_COL32(255, 255, 0, 255), 0.1f);
}

void DebugRenderer::drawTileMapAABBs(
    ImDrawList *drawList,
    const TileMap &tileMap,
    glm::vec2 cameraTopLeft,
    float zoom,
    glm::vec2 scale)
{
    auto tilePositions = tileMap.getTilePositionsAt(cameraTopLeft, glm::vec2(screenWidth, screenHeight));
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
            cameraTopLeft,
            zoom,
            scale,
            tile.isSpikes() ? IM_COL32(255, 0, 0, 255) : IM_COL32(0, 255, 0, 255));
    }
}

void DebugRenderer::drawAABB(
    ImDrawList *drawList,
    AABB aabb,
    glm::vec2 cameraTopLeft,
    float zoom,
    glm::vec2 scale,
    ImU32 color)
{
    if (aabb.isEmpty())
    {
        return;
    }

    glm::vec2 pos = (aabb.position - cameraTopLeft) * zoom;
    glm::vec2 size = aabb.size * zoom;
    ImVec2 topLeft = ImVec2(pos.x / scale.x, pos.y / scale.y);
    ImVec2 bottomRight = ImVec2((pos.x + size.x) / scale.x, (pos.y + size.y) / scale.y);
    drawList->AddRect(topLeft, bottomRight, color);
}

void DebugRenderer::update(float deltaTime)
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

void DebugRenderer::draw(
    const Camera2D &camera,
    const TileMap &tileMap,
    const Player &player)
{
    ImGuiIO &io = ImGui::GetIO();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImDrawList *drawList = ImGui::GetBackgroundDrawList();

    glm::vec2 cameraPosition = camera.getPosition();
    float zoom = camera.getZoom();
    glm::vec2 frameBufferSize(screenWidth, screenHeight);
    glm::vec2 cameraTopLeft = cameraPosition - frameBufferSize / (2.0f * zoom);
    glm::vec2 scale = frameBufferSize / glm::vec2(io.DisplaySize.x, io.DisplaySize.y);

    if (shouldDrawGrid)
    {
        drawGrid(drawList, tileMap, cameraPosition, cameraTopLeft, zoom, scale, io.DisplaySize.x, io.DisplaySize.y);
    }

    if (shouldDrawPlayerAABBs)
    {
        drawPlayerAABBs(drawList, player, cameraTopLeft, zoom, scale);
    }

    if (shouldDrawTileMapAABBs)
    {
        drawTileMapAABBs(drawList, tileMap, cameraTopLeft, zoom, scale);
    }

    for (const auto &[hash, debugAABB] : debugAABBs)
    {
        drawAABB(drawList, debugAABB.box, cameraTopLeft, zoom, scale, debugAABB.color);
    }

    if (showDebugControls)
    {
        ImGui::SetNextWindowSize(ImVec2(200, 100));
        ImGui::Begin("Debug Controls");
        if (ImGui::Button("Step"))
            onStep();
        ImGui::SameLine();
        if (ImGui::Button("Play"))
            onPlay();
        ImGui::End();
    }

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