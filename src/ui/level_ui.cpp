#include <string>
#include <cstddef>
#include <memory>
#include <optional>
#include <vector>
#include <tuple>
#include <variant>
#include <glaze/glaze.hpp>
#include "ui/level_ui.hpp"
#include "ui/debug_aabb_overlay.hpp"
#include "npc/npc_spawn_data.hpp"
#include "ui/actors_in_level.hpp"
#include "ui/save_controls.hpp"
#include "ui/tile_picker.hpp"
#include "ui/armed.hpp"
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

}

void LevelUi::draw(
    Level &level,
    const std::vector<std::unique_ptr<Npc>> &npcs,
    const ActorMotionState &playerMotionState,
    const glm::vec2 &playerFeet,
    const ActorState &playerState,
    const Texture2D &tileSet,
    const GameData &gameData,
    std::optional<Armed> &armed,
    EditorCommands &commands)
{
    if (ImGui::CollapsingHeader("State", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Text(
            "w%dxh%dxs%d",
            level.getTileMap().getWidth(),
            level.getTileMap().getHeight(),
            level.getTileMap().getTileSize());

        navigationUi.draw(level);
    }

    ImGui::Separator();
    drawOverlayToggles();

    ImGui::Separator();
    drawLevel(level, commands);

    ImGui::Separator();
    drawTiles(level, tileSet, gameData, armed);

    ImGui::Separator();
    if (ImGui::CollapsingHeader("Actors", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ActorShown wasShowing = showingActor;
        ActorAsked asked = drawActorsInLevel(
            level,
            npcs,
            playerMotionState,
            playerFeet,
            playerState,
            gameData.npcData,
            showingActor,
            armed);

        if (asked.addNpcOfType)
        {
            level.addNpc(
                NpcSpawnData{*asked.addNpcOfType, level.getPlayerStartTile(), std::nullopt});

            std::size_t placed = level.getNpcs().size() - 1;
            if (std::optional<PatrolData> run = level.runBeneathNpc(placed))
                level.setNpcPatrol(placed, *run);

            showingActor = ActorShown{ActorShown::What::Npc, placed};
        }
        else if (asked.removeShownNpc && showingActor.what == ActorShown::What::Npc)
        {
            level.removeNpc(showingActor.npcIndex);
            showingActor = ActorShown{};
        }
        else if (asked.clearShownBeat && showingActor.what == ActorShown::What::Npc)
            level.clearNpcPatrol(showingActor.npcIndex);
        else
            showingActor = asked.show;

        if (showingActor != wasShowing)
            armed.reset();
    }
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

void LevelUi::drawOverlayToggles()
{
    if (!ImGui::CollapsingHeader("Overlays", ImGuiTreeNodeFlags_DefaultOpen))
        return;

    ImGui::Checkbox("Info", &drawTileInfo);
    ImGui::SameLine();
    ImGui::Checkbox("Grid", &drawGrid);
    ImGui::Checkbox("Colliders", &drawTileColliders);
    ImGui::SameLine();
    ImGui::Checkbox("Bounds", &drawLevelBounds);
    navigationUi.drawOverlayToggles();
}

void LevelUi::drawTiles(
    Level &level,
    const Texture2D &tileSet,
    const GameData &gameData,
    std::optional<Armed> &armed)
{
    if (!ImGui::CollapsingHeader("Tiles", ImGuiTreeNodeFlags_DefaultOpen))
        return;

    std::vector<int> tileIndices = tilesToPickFrom(level.getTileMap());

    std::optional<int> showing = paintedTile(armed);
    std::optional<int> picked =
        drawTilePicker(tileSet, level.getTileMap().getTileSize(), tileIndices, showing);
    if (picked != showing)
        armed = picked ? std::optional<Armed>(PaintTile{*picked}) : std::nullopt;

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

    drawSpawnOf(imGuiManager, camera, level, showingActor);

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
    std::optional<Armed> &armed)
{
    if (!armed || imGuiManager.getIO().WantCaptureMouse)
        return;

    ImVec2 mouseScreenPosition = ImGui::GetMousePos();
    glm::vec2 worldPosition = imGuiManager.screenToWorld(
        mouseScreenPosition, camera.getZoom(), camera.getTopLeftPosition());
    glm::ivec2 tilePosition = level.getTileMap().tileContaining(worldPosition);
    if (!level.getTileMap().validTilePosition(tilePosition))
        return;

    if (const PaintTile *painting = std::get_if<PaintTile>(&*armed))
    {
        if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
            return;

        if (level.getTileMap().tilePositionToTileIndex(tilePosition) != painting->tileIndex)
        {
            level.getTileMap().setTileIndex(tilePosition, painting->tileIndex);
            level.rebuildGraphs();
        }

        return;
    }

    if (!ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        return;

    PickTile picking = std::get<PickTile>(*armed);
    if (picking.what != PickTile::For::PlayerStart && picking.npcIndex >= level.getNpcs().size())
    {
        armed.reset();
        return;
    }

    switch (picking.what)
    {
    case PickTile::For::PlayerStart:
        level.setPlayerStartTile(tilePosition);
        break;

    case PickTile::For::NpcSpawn:
        level.setNpcSpawnTile(picking.npcIndex, tilePosition);
        break;

    case PickTile::For::PatrolFrom:
    case PickTile::For::PatrolTo: {
        const NpcSpawnData &spawn = level.getNpcs()[picking.npcIndex];
        glm::ivec2 standing = level.beatEndAt(spawn, tilePosition);
        PatrolData patrol = spawn.patrol.value_or(PatrolData{standing, standing});
        if (picking.what == PickTile::For::PatrolFrom)
            patrol.from = standing;
        else
            patrol.to = standing;

        level.setNpcPatrol(picking.npcIndex, patrol);
        break;
    }
    }

    armed.reset();
}

void LevelUi::valuesReplaced()
{
    saveable.valuesReplaced();
}
