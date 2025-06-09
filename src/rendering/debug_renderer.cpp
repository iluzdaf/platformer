#include "rendering/debug_renderer.hpp"
#include "rendering/debug_renderer_data.hpp"
#include "rendering/texture2d.hpp"
#include "rendering/ui/imgui_manager.hpp"
#include "cameras/camera2d.hpp"
#include "game/tile_map/tile_map.hpp"
#include "game/player/player.hpp"

DebugRenderer::DebugRenderer(const DebugRendererData &data)
    : showDebugControls(data.showDebugControls),
      showTileMapControls(data.showTileMapControls)
{
}

void DebugRenderer::update(
    ImGuiManager &imGuiManager,
    const Camera2D &camera,
    TileMap &tileMap)
{
    ImVec2 mouseScreenPosition = ImGui::GetMousePos();
    glm::vec2 worldPosition = imGuiManager.screenToWorld(mouseScreenPosition, camera.getZoom(), camera.getTopLeftPosition());
    glm::ivec2 tilePosition = tileMap.worldToTilePosition(worldPosition);
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
    const TileMap &tileMap,
    const Texture2D &tileSet)
{
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(imGuiManager.getIO().DisplaySize);

    if (showDebugControls)
    {
        drawDebugControls();
    }

    if (showTileMapControls)
    {
        drawTileMapControls(tileMap, tileSet);
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
        onToggleDrawTileInfo();
    ImGui::SameLine();
    if (ImGui::Button("Grid"))
        onToggleDrawGrid();
    ImGui::SameLine();
    if (ImGui::Button("Player"))
        onToggleDrawPlayerAABBs();
    if (ImGui::Button("TileMap"))
        onToggleDrawTileMapAABBs();
    ImGui::End();
}