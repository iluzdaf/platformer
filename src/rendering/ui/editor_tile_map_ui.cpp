#include "rendering/ui/editor_tile_map_ui.hpp"
#include "rendering/texture2d.hpp"
#include "rendering/ui/imgui_manager.hpp"
#include "game/tile_map/tile_map.hpp"
#include "cameras/camera2d.hpp"

void EditorTileMapUi::draw(
    ImGuiManager &imGuiManager,
    const TileMap &tileMap,
    const Texture2D &tileSet,
    bool showTileMapEditor)
{
    if (!showTileMapEditor)
    {
        return;
    }

    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 200, 0));
    ImGui::SetNextWindowSize(ImVec2(200, 350));
    ImGui::Begin("TileMap Editor");

    std::string tileMapLevel = tileMap.getLevel();
    tileMapLevel = tileMapLevel.substr(tileMapLevel.find_last_of("/\\") + 1);
    ImGui::Text("%s w%dxh%dxs%d", tileMapLevel.c_str(), tileMap.getWidth(), tileMap.getHeight(), tileMap.getTileSize());

    if (!editing)
    {
        if (ImGui::Button("edit"))
            editing = true;

        ImGui::End();
        return;
    }

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
            selectedTileIndex = tileIndex;

        ImGui::GetWindowDrawList()->AddText(
            tilePos,
            IM_COL32(255, 255, 255, 255),
            std::to_string(tileIndex).c_str());

        if (previouslySelectedTileIndex == tileIndex)
            ImGui::PopStyleColor(3);

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

    ImGui::NewLine();
    if (ImGui::Button("Spawn"))
    {
        editingPlayerStartTile = true;
        selectedTileIndex = 0;
    }

    if (previouslyEditingPlayerStartTile)
        ImGui::PopStyleColor(3);

    ImGui::SameLine();
    if (ImGui::Button("Save"))
    {
        tileMap.save();
        editing = false;
    }

    ImGui::SameLine();
    if (ImGui::Button("Reload"))
        onLoadLevel(tileMap.getLevel());

    ImGui::End();
}

void EditorTileMapUi::update(
    ImGuiManager &imGuiManager,
    const Camera2D &camera,
    TileMap &tileMap)
{
    if (!editing)
    {
        return;
    }

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