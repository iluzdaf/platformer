#include <string>
#include <cstddef>
#include <optional>
#include <vector>
#include <tuple>
#include <glaze/glaze.hpp>
#include "ui/level_ui.hpp"
#include "ui/save_controls.hpp"
#include "ui/brush_picker.hpp"
#include "ui/brush.hpp"
#include "ui/editor_commands.hpp"
#include "ui/editor_section.hpp"
#include "rendering/texture2d.hpp"
#include "ui/imgui_manager.hpp"
#include "tile_map/tile_map.hpp"
#include "game/level.hpp"
#include "game/game_data.hpp"
#include "ui/data_inspector.hpp"
#include "tile_map/tile_data.hpp"
#include "tile_map/tile_palette.hpp"
#include "ui/tile_map_overlays.hpp"
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
    bool reverted = drawSaveControls(
        saveable,
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
        ImGui::Checkbox("Info", &drawTileInfo);
        ImGui::SameLine();
        ImGui::Checkbox("Grid", &drawGrid);
        ImGui::Checkbox("Colliders", &drawTileColliders);
        ImGui::SameLine();
        ImGui::Checkbox("Bounds", &drawLevelBounds);
        ImGui::SameLine();
        ImGui::Checkbox("Spawn", &drawPlayerStart);
    }
    ImGui::Separator();

    std::vector<Brush> brushes;
    for (const auto &[tileIndex, tile] : level.getTileMap().getTiles())
        brushes.push_back(Brush{Brush::Kind::Tile, tileIndex});
    brushes.push_back(Brush{Brush::Kind::PlayerStart, 0});

    brush = drawBrushPicker(tileSet, level.getTileMap().getTileSize(), brushes, brush);
    std::optional<int> picked = tileOf(brush);

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

bool LevelUi::hasUnsavedChanges(const Level &level) const
{
    std::string json;
    std::ignore = glz::write_json(level.toLevelData(), json);
    return saveable.unsaved(level.getPath(), json);
}

bool LevelUi::drawsTileColliders() const
{
    return drawTileColliders;
}

bool LevelUi::drawsLevelBounds() const
{
    return drawLevelBounds;
}

bool LevelUi::drawsPlayerStart() const
{
    return drawPlayerStart;
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
