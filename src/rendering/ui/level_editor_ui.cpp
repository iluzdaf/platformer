#include <string>
#include <cstddef>
#include <optional>
#include <vector>
#include <tuple>
#include <glaze/glaze.hpp>
#include "rendering/ui/level_editor_ui.hpp"
#include "rendering/ui/tile_picker.hpp"
#include "rendering/ui/editor_commands.hpp"
#include "rendering/ui/editor_section.hpp"
#include "rendering/texture2d.hpp"
#include "rendering/ui/imgui_manager.hpp"
#include "tile_map/tile_map.hpp"
#include "game/level.hpp"
#include "game/game_data.hpp"
#include "rendering/ui/data_inspector.hpp"
#include "tile_map/tile_data.hpp"
#include "tile_map/tile_palette.hpp"
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
    const std::string &firstLevel,
    const GameData &gameData,
    std::optional<int> &selectedTileIndex,
    EditorCommands &commands)
{
    switch (section)
    {
    case EditorSection::Level:
        drawLevel(level, firstLevel, commands);
        break;

    case EditorSection::TileMap:
        drawTileMap(level, tileSet, gameData, selectedTileIndex);
        break;

    default:
        break;
    }
}

void LevelEditorUi::drawLevel(Level &level, const std::string &firstLevel, EditorCommands &commands)
{
    std::string json;
    std::ignore = glz::write_json(level.toLevelData(), json);
    bool reverted = saveable.drawControls(
        level.getPath(),
        json,
        [&level] { level.save(); },
        [&](const std::string &) { commands.onLoadLevel(level.getPath()); });
    if (reverted)
        return;

    ImGui::Separator();

    std::optional<std::string> chosenLevel = drawLevelChooser(level, firstLevel, commands);
    if (chosenLevel)
    {
        commands.onLoadLevel(*chosenLevel);
        return;
    }

    ImGui::TextUnformatted("next");
    ImGui::SameLine();
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

void LevelEditorUi::drawTileMap(
    Level &level,
    const Texture2D &tileSet,
    const GameData &gameData,
    std::optional<int> &selectedTileIndex)
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

    std::vector<int> tileIndices;
    for (const auto &[tileIndex, tile] : level.getTileMap().getTiles())
        tileIndices.push_back(tileIndex);

    selectedTileIndex =
        drawTilePicker(tileSet, level.getTileMap().getTileSize(), tileIndices, selectedTileIndex);

    const TilePalette &palette = gameData.tilePalettes.at(level.getTileMap().getTilePalette());
    if (selectedTileIndex && palette.contains(*selectedTileIndex))
    {
        ImGui::Separator();
        TileData shown = palette.at(*selectedTileIndex);
        ImGui::BeginDisabled();
        inspector::drawFields(shown);
        ImGui::EndDisabled();
        ImGui::Separator();
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
        selectedTileIndex.reset();
    }

    if (previouslyEditingPlayerStartTile)
        ImGui::PopStyleColor(3);
}

std::optional<std::string> LevelEditorUi::drawLevelChooser(
    const Level &level,
    const std::string &firstLevel,
    EditorCommands &commands)
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

    ImGui::SameLine();
    bool isFirst = level.getPath() == firstLevel;
    if (ImGui::Checkbox("first", &isFirst) && isFirst)
        commands.onSetFirstLevel();

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

void LevelEditorUi::update(
    const ImGuiManager &imGuiManager,
    const Camera2D &camera,
    Level &level,
    std::optional<int> selectedTileIndex)
{
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
        else if (
            selectedTileIndex &&
            level.getTileMap().tilePositionToTileIndex(tilePosition) != *selectedTileIndex)
        {
            level.getTileMap().setTileIndex(tilePosition, *selectedTileIndex);
            level.rebuildGraphs();
        }
    }
}

void LevelEditorUi::valuesReplaced()
{
    saveable.valuesReplaced();
}
