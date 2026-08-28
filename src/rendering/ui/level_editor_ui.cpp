#include <string>
#include <cstddef>
#include <optional>
#include <cstdint>
#include <tuple>
#include <utility>
#include "rendering/ui/level_editor_ui.hpp"
#include "rendering/ui/editor_section.hpp"
#include "rendering/texture2d.hpp"
#include "rendering/ui/imgui_manager.hpp"
#include "tile_map/tile_map.hpp"
#include "game/level.hpp"
#include "rendering/ui/tile_map_overlays.hpp"
#include "game/levels.hpp"
#include "cameras/camera2d.hpp"

namespace
{
    std::string levelName(const std::string &levelPath)
    {
        std::string name = levelPath.substr(levelPath.find_last_of("/\\") + 1);
        size_t extension = name.rfind(".json");
        return extension == std::string::npos ? name : name.substr(0, extension);
    }
}

void LevelEditorUi::draw(
    EditorSection section,
    Level &level,
    const Texture2D &tileSet,
    const std::string &firstLevel)
{
    switch (section)
    {
    case EditorSection::Level:
        drawLevel(level, firstLevel);
        break;

    case EditorSection::TileMap:
        drawTileMap(level, tileSet);
        break;

    default:
        break;
    }
}

void LevelEditorUi::drawLevel(Level &level, const std::string &firstLevel)
{
    if (!editing)
    {
        if (ImGui::Button("edit"))
            editing = true;
    }
    else
    {
        if (ImGui::Button("save"))
        {
            level.save();
            editing = false;
        }

        ImGui::SameLine();
        if (ImGui::Button("cancel"))
        {
            editing = false;
            onLoadLevel(level.getPath());
            return;
        }
    }

    ImGui::Separator();

    std::optional<std::string> chosenLevel = drawLevelChooser(level, firstLevel);
    if (chosenLevel)
    {
        editing = false;
        onLoadLevel(*chosenLevel);
        return;
    }

    ImGui::TextUnformatted("next");
    ImGui::SameLine();
    if (!editing)
        ImGui::TextUnformatted(levelName(level.getNextLevel()).c_str());
    else
    {
        ImGui::SetNextItemWidth(110.0f);
        if (ImGui::BeginCombo("##next", levelName(level.getNextLevel()).c_str()))
        {
            std::string directory = level.getPath().substr(0, level.getPath().find_last_of("/\\"));
            for (const std::string &path : levelPathsIn(directory))
                if (ImGui::Selectable(levelName(path).c_str(), path == level.getNextLevel()))
                    level.setNextLevel(path);

            ImGui::EndCombo();
        }
    }
}

void LevelEditorUi::drawTileMap(Level &level, const Texture2D &tileSet)
{
    ImGui::Text(
        "w%dxh%dxs%d",
        level.getTileMap().getWidth(),
        level.getTileMap().getHeight(),
        level.getTileMap().getTileSize());

    ImGui::Checkbox("Tile Info", &drawTileInfo);
    ImGui::SameLine();
    ImGui::Checkbox("Grid", &drawGrid);
    ImGui::Checkbox("AABBs", &drawTileMapAABBs);
    ImGui::Separator();

    if (!editing)
    {
        ImGui::TextDisabled("editing the level is off");
        return;
    }

    const auto &tiles = level.getTileMap().getTiles();
    int columns = 4;
    int count = 0;
    int tileSize = level.getTileMap().getTileSize();
    ImTextureID imguiTextureID = (ImTextureID)(intptr_t)tileSet.getTextureID();

    for (const auto &[tileIndex, tile] : tiles)
    {
        ImGui::PushID(tileIndex);
        auto [uvStart, uvEnd] = tileSet.getUVRange(tileIndex, tileSize, false);
        int previouslySelectedTileIndex = selectedTileIndex;

        if (previouslySelectedTileIndex == tileIndex)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(255, 255, 0, 255));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(255, 255, 0, 255));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(255, 255, 0, 255));
        }

        ImVec2 tilePos = ImGui::GetCursorScreenPos();
        if (ImGui::ImageButton(
                "##tile",
                imguiTextureID,
                ImVec2(32, 32),
                ImVec2(uvStart.x, uvStart.y),
                ImVec2(uvEnd.x, uvEnd.y)))
            selectedTileIndex = tileIndex;

        ImGui::GetWindowDrawList()->AddText(
            tilePos, IM_COL32(255, 255, 255, 255), std::to_string(tileIndex).c_str());

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
}

std::optional<std::string> LevelEditorUi::drawLevelChooser(
    const Level &level,
    const std::string &firstLevel)
{
    std::string directory = level.getPath().substr(0, level.getPath().find_last_of("/\\"));
    std::optional<std::string> chosen;

    ImGui::SetNextItemWidth(110.0f);
    if (ImGui::BeginCombo("##level", levelName(level.getPath()).c_str()))
    {
        for (const std::string &path : levelPathsIn(directory))
        {
            bool current = path == level.getPath();
            if (ImGui::Selectable(levelName(path).c_str(), current) && !current)
                chosen = path;
        }
        ImGui::EndCombo();
    }

    if (editing)
    {
        ImGui::SameLine();
        bool isFirst = level.getPath() == firstLevel;
        if (ImGui::Checkbox("first", &isFirst) && isFirst)
            onSetFirstLevel();
    }

    return chosen;
}

void LevelEditorUi::drawOverlay(
    const ImGuiManager &imGuiManager,
    const Camera2D &camera,
    const Level &level) const
{
    if (drawGrid)
        drawTileGrid(imGuiManager, camera, level.getTileMap());

    if (drawTileInfo)
        ::drawTileInfo(imGuiManager, camera, level.getTileMap());
}

bool LevelEditorUi::drawsTileMapAABBs() const
{
    return drawTileMapAABBs;
}

void LevelEditorUi::update(const ImGuiManager &imGuiManager, const Camera2D &camera, Level &level)
{
    if (!editing)
        return;

    ImVec2 mouseScreenPosition = ImGui::GetMousePos();
    glm::vec2 worldPosition = imGuiManager.screenToWorld(
        mouseScreenPosition, camera.getZoom(), camera.getTopLeftPosition());
    glm::ivec2 tilePosition = level.getTileMap().worldToTilePosition(worldPosition);
    if (!level.getTileMap().validTilePosition(tilePosition))
        return;

    if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && !imGuiManager.getIO().WantCaptureMouse)
    {
        if (editingPlayerStartTile)
        {
            level.setPlayerStartTile(tilePosition);
            editingPlayerStartTile = false;
        }
        else if (level.getTileMap().tilePositionToTileIndex(tilePosition) != selectedTileIndex)
        {
            level.getTileMap().setTileIndex(tilePosition, selectedTileIndex);
            level.rebuildGraphs();
        }
    }
}