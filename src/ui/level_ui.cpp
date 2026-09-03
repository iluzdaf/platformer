#include <map>
#include <stdexcept>
#include <string>
#include <cstddef>
#include <optional>
#include <vector>
#include <variant>
#include <glaze/glaze.hpp>
#include "ui/level_ui.hpp"
#include "game/level_data_file.hpp"
#include "ui/mouse_on_the_map.hpp"
#include "ui/debug_aabb_overlay.hpp"
#include "npc/npc_spawn_data.hpp"
#include "npc/npc.hpp"
#include "ui/actors_in_level.hpp"
#include "ui/armed.hpp"
#include "ui/editor_commands.hpp"
#include "ui/grid_shown.hpp"
#include "ui/imgui_manager.hpp"
#include "tile_map/tile_map.hpp"
#include "game/level.hpp"
#include "game/catalogue.hpp"
#include "npc/npc_data.hpp"
#include "ui/tile_map_overlays.hpp"
#include "game/levels.hpp"
#include "cameras/camera2d.hpp"

void LevelUi::draw(
    Level &level,
    const std::string &levelPath,
    const ActorMotionState &playerMotionState,
    const glm::vec2 &playerFeet,
    const ActorState &playerState,
    const std::map<std::string, NpcData> &npcData,
    std::optional<Armed> &armed,
    EditorCommands &commands)
{
    if (ImGui::CollapsingHeader("State"))
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
    if (!ImGui::CollapsingHeader("Inspector"))
        return;

    drawLevel(level, levelPath);
    drawActors(level, playerMotionState, playerFeet, playerState, npcData, armed, commands);
}

void LevelUi::drawActors(
    Level &level,
    const ActorMotionState &playerMotionState,
    const glm::vec2 &playerFeet,
    const ActorState &playerState,
    const std::map<std::string, NpcData> &npcData,
    std::optional<Armed> &armed,
    EditorCommands &commands)
{
    if (!ImGui::TreeNode("Actors"))
        return;

    ActorShown wasShowing = showingActor;
    ActorAsked asked = drawActorsInLevel(
        level, playerMotionState, playerFeet, playerState, npcData, showingActor, armed);

    if (asked.addNpcOfType)
    {
        level.addNpc(
            NpcSpawnData{*asked.addNpcOfType, level.getPlayerStartTile(), std::nullopt},
            oneNamed(npcData, "npc", *asked.addNpcOfType));

        std::size_t placed = level.getNpcs().size() - 1;
        if (std::optional<PatrolData> run = level.runBeneathNpc(placed))
            level.setNpcPatrol(placed, *run);

        showingActor = ActorShown{ActorShown::What::Npc, placed};
        commands.onNpcsChanged();
    }
    else if (asked.removeShownNpc && showingActor.what == ActorShown::What::Npc)
    {
        level.removeNpc(showingActor.npcIndex);
        showingActor = ActorShown{};
        commands.onNpcsChanged();
    }
    else if (asked.clearShownBeat && showingActor.what == ActorShown::What::Npc)
    {
        level.clearNpcPatrol(showingActor.npcIndex);
        commands.onNpcsChanged();
    }
    else
        showingActor = asked.show;

    if (showingActor != wasShowing)
        armed.reset();

    ImGui::TreePop();
}

std::string LevelUi::asItWouldBeSaved(const Level &level) const
{
    std::string json;
    if (glz::write_json(level.toLevelData(), json))
        throw std::runtime_error("Failed to serialise the level for comparison");

    return json;
}

void LevelUi::drawLevel(Level &level, const std::string &levelPath)
{
    ImGui::TextUnformatted("next");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(110.0f);
    if (ImGui::BeginCombo("##next", levelName(level.getNextLevel()).c_str()))
    {
        std::string directory = directoryOf(levelPath);
        for (const std::string &path : levelPathsIn(directory))
            if (ImGui::Selectable(levelName(path).c_str(), path == level.getNextLevel()))
                level.setNextLevel(path);

        ImGui::EndCombo();
    }
}

void LevelUi::drawOverlayToggles()
{
    if (!ImGui::CollapsingHeader("Overlays"))
        return;

    ImGui::Checkbox("Info", &drawTileInfo);
    ImGui::SameLine();
    ImGui::Checkbox("Grid", &grid.showing);
    ImGui::Checkbox("Colliders", &drawTileColliders);
    ImGui::SameLine();
    ImGui::Checkbox("Bounds", &drawLevelBounds);
    navigationUi.drawOverlayToggles();
}

void LevelUi::drawOverlay(
    const ImGuiManager &imGuiManager,
    const Camera2D &camera,
    const Level &level) const
{
    if (grid.showing)
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

void LevelUi::save(const Level &level, const std::string &levelPath)
{
    writeLevelData(level.toLevelData(), levelPath);
    saveable.saved(levelPath, asItWouldBeSaved(level));
}

bool LevelUi::unsavedSince(const Level &level, const std::string &levelPath)
{
    return saveable.unsavedSince(levelPath, asItWouldBeSaved(level));
}

void LevelUi::update(
    const MouseOnTheMap &mouse,
    Level &level,
    const std::string &levelPath,
    std::optional<Armed> &armed,
    EditorCommands &commands)
{
    if (saveable.lastSeen(levelPath).empty())
        saveable.seen(levelPath, asItWouldBeSaved(level));

    grid = whileArmed(grid, armed.has_value());

    if (!armed || mouse.overTheUi)
        return;

    glm::ivec2 tilePosition = level.getTileMap().tileContaining(mouse.worldPosition);
    if (!level.getTileMap().validTilePosition(tilePosition))
        return;

    if (const PaintTile *painting = std::get_if<PaintTile>(&*armed))
    {
        if (!mouse.heldDown)
            return;

        if (level.getTileMap().tilePositionToTileIndex(tilePosition) != painting->tileIndex)
        {
            level.getTileMap().setTileIndex(tilePosition, painting->tileIndex);
            level.rebuildGraphs();
        }

        return;
    }

    if (!mouse.justClicked)
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
        commands.onNpcsChanged();
        break;

    case PickTile::For::PatrolFrom:
    case PickTile::For::PatrolTo: {
        const NpcSpawnData &spawn = level.getNpcs()[picking.npcIndex]->getSpawn();
        PatrolData patrol = spawn.patrol.value_or(PatrolData{tilePosition, tilePosition});
        if (picking.what == PickTile::For::PatrolFrom)
            patrol.from = tilePosition;
        else
            patrol.to = tilePosition;

        level.setNpcPatrol(picking.npcIndex, patrol);
        commands.onNpcsChanged();
        break;
    }
    }

    armed.reset();
}

void LevelUi::valuesReplaced()
{
    saveable.valuesReplaced();
}
