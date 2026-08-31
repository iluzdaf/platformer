#include <string>
#include <cstddef>
#include <memory>
#include <optional>
#include <vector>
#include <tuple>
#include <glaze/glaze.hpp>
#include "ui/level_ui.hpp"
#include "ui/debug_aabb_overlay.hpp"
#include "ui/npcs_in_level.hpp"
#include "ui/save_controls.hpp"
#include "ui/brush_picker.hpp"
#include "ui/brush.hpp"
#include "ui/editor_commands.hpp"
#include "rendering/texture2d.hpp"
#include "ui/imgui_manager.hpp"
#include "tile_map/tile_map.hpp"
#include "game/level.hpp"
#include "game/game_data.hpp"
#include "tile_map/tile_data.hpp"
#include "tile_map/tile_palette.hpp"
#include "ui/tile_map_overlays.hpp"
#include "game/levels.hpp"
#include "cameras/camera2d.hpp"

namespace
{
    void drawTileSummary(const TileData &tile)
    {
        std::string flags;
        auto note = [&flags](bool set, const char *name)
        {
            if (!set)
                return;

            if (!flags.empty())
                flags += "  ";
            flags += name;
        };

        note(tile.solid, "solid");
        note(tile.deadly, "deadly");
        note(tile.portal, "portal");
        note(tile.grippable, "grippable");

        if (!flags.empty())
            ImGui::TextUnformatted(flags.c_str());

        if (tile.collider)
            ImGui::Text(
                "collider %.0f,%.0f %.0fx%.0f",
                tile.collider->offset.x,
                tile.collider->offset.y,
                tile.collider->size.x,
                tile.collider->size.y);

        if (tile.pickup && tile.pickup->scoreDelta)
            ImGui::Text(
                "pickup leaves %d, scores %d", tile.pickup->replaceIndex, *tile.pickup->scoreDelta);
        else if (tile.pickup)
            ImGui::Text("pickup leaves %d", tile.pickup->replaceIndex);

        if (tile.animationData)
            ImGui::Text(
                "animates %d frames at %.2fs",
                static_cast<int>(tile.animationData->frameAnimationData.frames.size()),
                tile.animationData->frameAnimationData.frameDuration);

        if (flags.empty() && !tile.collider && !tile.pickup && !tile.animationData)
            ImGui::TextDisabled("nothing set");
    }

    std::optional<int> tileOf(const std::optional<Brush> &brush)
    {
        if (!brush || brush->kind != Brush::Kind::Tile)
            return std::nullopt;

        return brush->tileIndex;
    }
}

void LevelUi::draw(
    Level &level,
    const std::vector<std::unique_ptr<Npc>> &npcs,
    const Texture2D &tileSet,
    const GameData &gameData,
    std::optional<Brush> &brush,
    EditorCommands &commands)
{
    drawTileMap(level, tileSet, gameData, brush);

    ImGui::Separator();
    if (ImGui::CollapsingHeader("Navigation", ImGuiTreeNodeFlags_DefaultOpen))
        navigationUi.draw(level);

    ImGui::Separator();
    drawLevel(level, commands);

    ImGui::Separator();
    if (ImGui::CollapsingHeader("NPCs", ImGuiTreeNodeFlags_DefaultOpen))
        drawNpcsInLevel(level, npcs);
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
        navigationUi.drawOverlayToggles();
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
        drawTileSummary(palette.at(*picked));
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

    if (drawTileColliders)
        ::drawTileColliders(imGuiManager, camera, level);

    if (drawLevelBounds)
        ::drawLevelBounds(imGuiManager, camera, level);

    if (drawPlayerStart)
        ::drawPlayerStart(imGuiManager, camera, level);

    navigationUi.drawOverlay(imGuiManager, camera, level);
}

bool LevelUi::hasUnsavedChanges(const Level &level) const
{
    std::string json;
    std::ignore = glz::write_json(level.toLevelData(), json);
    return saveable.unsaved(level.getPath(), json);
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
