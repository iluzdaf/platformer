#include <string>
#include <cstddef>
#include <optional>
#include <vector>
#include <tuple>
#include <glaze/glaze.hpp>
#include "rendering/ui/level_ui.hpp"
#include "rendering/ui/tile_picker.hpp"
#include "rendering/ui/brush.hpp"
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
    std::optional<int> tileOf(const std::optional<Brush> &brush)
    {
        if (!brush || brush->kind != Brush::Kind::Tile)
            return std::nullopt;

        return brush->tileIndex;
    }
}

void LevelUi::draw(
    EditorSection section,
    Level &level,
    const Texture2D &tileSet,
    const GameData &gameData,
    std::optional<Brush> &brush,
    EditorCommands &commands)
{
    if (section != EditorSection::Level)
        return;

    drawTileMap(level, tileSet, gameData, brush);
    ImGui::Separator();
    drawLevel(level, commands);
}

void LevelUi::drawLevel(Level &level, EditorCommands &commands)
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

    ImGui::TextUnformatted("next");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(110.0f);
    if (ImGui::BeginCombo("##next", levelName(level.getNextLevel()).c_str()))
    {
        std::string directory = directoryOf(level.getPath());
        for (const std::string &path : levelPathsIn(directory))
            if (ImGui::Selectable(levelName(path).c_str(), path == level.getNextLevel()))
                level.setNextLevel(path);

        ImGui::EndCombo();
    }
}

void LevelUi::drawTileMap(
    Level &level,
    const Texture2D &tileSet,
    const GameData &gameData,
    std::optional<Brush> &brush)
{
    ImGui::Text(
        "w%dxh%dxs%d",
        level.getTileMap().getWidth(),
        level.getTileMap().getHeight(),
        level.getTileMap().getTileSize());

    if (ImGui::CollapsingHeader("Overlays", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Checkbox("Tile Info", &drawTileInfo);
        ImGui::SameLine();
        ImGui::Checkbox("Grid", &drawGrid);
        ImGui::Checkbox("AABBs", &drawTileMapAABBs);
    }
    ImGui::Separator();

    std::vector<int> tileIndices;
    for (const auto &[tileIndex, tile] : level.getTileMap().getTiles())
        tileIndices.push_back(tileIndex);

    std::optional<int> paintingTile = tileOf(brush);
    std::optional<int> picked =
        drawTilePicker(tileSet, level.getTileMap().getTileSize(), tileIndices, paintingTile);
    if (picked != paintingTile)
        brush = picked ? std::optional<Brush>(Brush{Brush::Kind::Tile, *picked}) : std::nullopt;

    bool placingPlayerStart = brush && brush->kind == Brush::Kind::PlayerStart;
    if (placingPlayerStart)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, TilePickerArmedColour);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, TilePickerArmedColour);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, TilePickerArmedColour);
    }

    ImVec2 padding = ImGui::GetStyle().FramePadding;
    if (ImGui::Button(
            "spawn",
            ImVec2(TilePickerCellSize + padding.x * 2.0f, TilePickerCellSize + padding.y * 2.0f)))
        brush = placingPlayerStart ? std::nullopt
                                   : std::optional<Brush>(Brush{Brush::Kind::PlayerStart, 0});

    if (placingPlayerStart)
        ImGui::PopStyleColor(3);

    const TilePalette &palette = gameData.tilePalettes.at(level.getTileMap().getTilePalette());
    if (picked && palette.contains(*picked))
    {
        ImGui::Separator();
        TileData shown = palette.at(*picked);
        ImGui::BeginDisabled();
        inspector::drawFields(shown);
        ImGui::EndDisabled();
        ImGui::Separator();
    }
}

void LevelUi::drawOverlay(
    const ImGuiManager &imGuiManager,
    const Camera2D &camera,
    const Level &level) const
{
    if (drawGrid)
        drawTileGrid(imGuiManager, camera, level.getTileMap());

    if (drawTileInfo)
        ::drawTileInfo(imGuiManager, camera, level.getTileMap());
}

bool LevelUi::drawsTileMapAABBs() const
{
    return drawTileMapAABBs;
}

void LevelUi::update(
    const ImGuiManager &imGuiManager,
    const Camera2D &camera,
    Level &level,
    std::optional<Brush> &brush)
{
    ImVec2 mouseScreenPosition = ImGui::GetMousePos();
    glm::vec2 worldPosition = imGuiManager.screenToWorld(
        mouseScreenPosition, camera.getZoom(), camera.getTopLeftPosition());
    glm::ivec2 tilePosition = level.getTileMap().worldToTilePosition(worldPosition);
    if (!level.getTileMap().validTilePosition(tilePosition))
        return;

    if (!brush || !ImGui::IsMouseDown(ImGuiMouseButton_Left) ||
        imGuiManager.getIO().WantCaptureMouse)
        return;

    switch (brush->kind)
    {
    case Brush::Kind::PlayerStart:
        level.setPlayerStartTile(tilePosition);
        brush.reset();
        break;

    case Brush::Kind::Tile:
        if (level.getTileMap().tilePositionToTileIndex(tilePosition) != brush->tileIndex)
        {
            level.getTileMap().setTileIndex(tilePosition, brush->tileIndex);
            level.rebuildGraphs();
        }
        break;
    }
}

void LevelUi::valuesReplaced()
{
    saveable.valuesReplaced();
}
