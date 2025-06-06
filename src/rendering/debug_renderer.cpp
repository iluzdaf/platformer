#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <glm/gtc/matrix_transform.hpp>
#include "rendering/debug_renderer.hpp"
#include "cameras/camera2d.hpp"
#include "game/tile_map/tile_map.hpp"
#include "game/player/player.hpp"

DebugRenderer::DebugRenderer(
    GLFWwindow *window,
    float screenWidth,
    float screenHeight)
    : screenWidth(screenWidth),
      screenHeight(screenHeight)
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

    glm::vec2 camPos = camera.getPosition();
    float zoom = camera.getZoom();
    glm::vec2 frameBufferSize(screenWidth, screenHeight);
    glm::vec2 camTopLeft = camPos - frameBufferSize / (2.0f * zoom);
    glm::vec2 scale = frameBufferSize / glm::vec2(io.DisplaySize.x, io.DisplaySize.y);
    float scaledTileInFramebuffer = tileMap.getTileSize() * zoom;
    float offsetX_fb = fmod(camPos.x * zoom, scaledTileInFramebuffer);
    float offsetY_fb = fmod(camPos.y * zoom, scaledTileInFramebuffer);

    float rawOffsetX = fmod(camTopLeft.x * zoom, scaledTileInFramebuffer);
    if (rawOffsetX < 0.0f)
        rawOffsetX += scaledTileInFramebuffer;
    float offsetX = -rawOffsetX / scale.x;
    float rawOffsetY = fmod(camTopLeft.y * zoom, scaledTileInFramebuffer);
    if (rawOffsetY < 0.0f)
        rawOffsetY += scaledTileInFramebuffer;
    float offsetY = -rawOffsetY / scale.y;

    float tileSizeImgui = scaledTileInFramebuffer / scale.x;

    for (float x = offsetX; x < io.DisplaySize.x; x += tileSizeImgui)
    {
        drawList->AddLine(ImVec2(x, 0), ImVec2(x, io.DisplaySize.y), IM_COL32(100, 100, 100, 255));
    }

    for (float y = offsetY; y < io.DisplaySize.y; y += tileSizeImgui)
    {
        drawList->AddLine(ImVec2(0, y), ImVec2(io.DisplaySize.x, y), IM_COL32(100, 100, 100, 255));
    }

    AABB box = player.getAABB();
    glm::vec2 pos = (box.position - camTopLeft) * zoom;
    glm::vec2 size = box.size * zoom;
    ImVec2 topLeft = ImVec2(pos.x / scale.x, pos.y / scale.y);
    ImVec2 bottomRight = ImVec2((pos.x + size.x) / scale.x, (pos.y + size.y) / scale.y);
    drawList->AddRect(topLeft, bottomRight, IM_COL32(0, 255, 0, 255));

    auto tilePositions = tileMap.getTilePositionsAt(camTopLeft, frameBufferSize);
    for (auto tilePosition : tilePositions)
    {
        auto tile = tileMap.getTile(tilePosition);
        if (tile.isSpikes() || tile.isPickup())
        {
            glm::vec2 tileWorldPosition = tileMap.getTileWorldPosition(tilePosition);
            AABB box = tile.getAABBAt(tileWorldPosition);
            glm::vec2 pos = (box.position - camTopLeft) * zoom;
            glm::vec2 size = box.size * zoom;
            ImVec2 topLeft = ImVec2(pos.x / scale.x, pos.y / scale.y);
            ImVec2 bottomRight = ImVec2((pos.x + size.x) / scale.x, (pos.y + size.y) / scale.y);
            drawList->AddRect(topLeft, bottomRight, tile.isSpikes() ? IM_COL32(255, 0, 0, 255) : IM_COL32(0, 255, 0, 255));
        }
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void DebugRenderer::resize(float screenWidth, float screenHeight)
{
    this->screenWidth = screenWidth;
    this->screenHeight = screenHeight;
}