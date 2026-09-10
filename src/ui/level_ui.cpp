#include <map>
#include "game/level_data.hpp"
#include <utility>
#include <stdexcept>
#include <string>
#include <cstddef>
#include <optional>
#include <vector>
#include <variant>
#include <glaze/glaze.hpp>
#include "ui/level_ui.hpp"
#include "ui/file_chooser.hpp"
#include "actor/actor_animation_data.hpp"
#include "game/level_data_file.hpp"
#include "ui/mouse_on_the_map.hpp"
#include "ui/debug_aabb_overlay.hpp"
#include "npc/npc_spawn_data.hpp"
#include "game/beat_between.hpp"
#include "ui/actors_in_level.hpp"
#include "ui/armed.hpp"
#include "ui/editor_commands.hpp"
#include "ui/tile_map_shown.hpp"
#include "ui/imgui_manager.hpp"
#include "tile_map/tile_map.hpp"
#include "game/level.hpp"
#include "navigation/navigation_profile_builder.hpp"
#include "game/catalogue.hpp"
#include "npc/npc_data.hpp"
#include "ui/tile_map_overlays.hpp"
#include "ui/size_buttons.hpp"
#include "game/level_resizing.hpp"
#include "game/levels.hpp"
#include "cameras/camera2d.hpp"

void LevelUi::draw(
    const Level &level,
    const LevelData &levelData,
    const std::string &levelPath,
    const ActorAnimationData &playerAnimations,
    const Observed &playerObserved,
    const glm::vec2 &playerFeet,
    const ActorState &playerState,
    const std::map<std::string, NpcData> &npcData,
    std::optional<Armed> &armed,
    EditorCommands &commands)
{
    drawLevel(level, levelData, levelPath, commands);
    navigationUi.draw(level);
    drawActors(
        level,
        levelData,
        playerAnimations,
        playerObserved,
        playerFeet,
        playerState,
        npcData,
        armed,
        commands);
}

void LevelUi::drawActors(
    const Level &level,
    const LevelData &levelData,
    const ActorAnimationData &playerAnimations,
    const Observed &playerObserved,
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
        level,
        playerAnimations,
        playerObserved,
        playerFeet,
        playerState,
        npcData,
        showingActor,
        armed);

    if (asked.addNpcOfType)
    {
        const NpcData &kind = oneNamed(npcData, "npc", *asked.addNpcOfType);
        NpcSpawnData placing{*asked.addNpcOfType, levelData.playerFeet, std::nullopt};
        placing.patrol = level.runBeneath(buildNavigationProfile(kind.actorData), placing.feet);

        LevelData edited = levelData;
        edited.npcs.push_back(placing);

        showingActor = ActorShown{ActorShown::What::Npc, edited.npcs.size() - 1};
        commands.onLevelEdited(edited);
    }
    else if (asked.removeShownNpc && showingActor.what == ActorShown::What::Npc)
    {
        LevelData edited = levelData;
        edited.npcs.erase(edited.npcs.begin() + static_cast<std::ptrdiff_t>(showingActor.npcIndex));

        showingActor = ActorShown{};
        commands.onLevelEdited(edited);
    }
    else if (asked.clearShownBeat && showingActor.what == ActorShown::What::Npc)
    {
        LevelData edited = levelData;
        edited.npcs[showingActor.npcIndex].patrol.reset();
        commands.onLevelEdited(edited);
    }
    else
        showingActor = asked.show;

    if (showingActor != wasShowing)
        armed.reset();

    ImGui::TreePop();
}

std::string LevelUi::asItWouldBeSaved(const LevelData &levelData) const
{
    std::string json;
    if (glz::write_json(levelData, json))
        throw std::runtime_error("Failed to serialise the level for comparison");

    return json;
}

void askedToResize(
    Resize resize,
    const LevelData &levelData,
    int tileSize,
    EditorCommands &commands)
{
    commands.onLevelResized(resizedBy(resize, levelData, tileSize), shiftOf(resize, tileSize));
}

void LevelUi::drawLevel(
    const Level &level,
    const LevelData &levelData,
    const std::string &levelPath,
    EditorCommands &commands)
{
    const TileMap &tileMap = level.getTileMap();
    if (std::optional<Resize> resize = drawSizeButtons(tileMap.getWidth(), tileMap.getHeight()))
        askedToResize(*resize, levelData, tileMap.getTileSize(), commands);

    LevelData edited = levelData;
    if (drawFileChooser("next", edited.nextLevel.path, directoryOf(levelPath), ".json"))
        commands.onLevelEdited(edited);
}

void LevelUi::drawOverlayToggles()
{
    ImGui::Checkbox("Tile map", &tileMapShown.showing);
    ImGui::Checkbox("Npcs", &npcsShown);
    navigationUi.drawOverlayToggles();
}

void LevelUi::drawOverlay(
    const ImGuiManager &imGuiManager,
    const Camera2D &camera,
    const Level &level) const
{
    if (tileMapShown.showing)
    {
        drawTileGrid(imGuiManager, camera, level.getTileMap());
        drawTileInfo(imGuiManager, camera, level.getTileMap());
        drawTileColliders(imGuiManager, camera, level);
        drawLevelBounds(imGuiManager, camera, level);
    }

    if (npcsShown)
        drawNpcColliders(imGuiManager, camera, level);

    drawSpawnOf(imGuiManager, camera, level, showingActor);

    navigationUi.drawOverlay(imGuiManager, camera, level);
}

void LevelUi::save(const LevelData &levelData, const std::string &levelPath)
{
    writeLevelData(levelData, levelPath);
    saveable.saved(levelPath, asItWouldBeSaved(levelData));
}

bool LevelUi::unsavedSince(const LevelData &levelData, const std::string &levelPath)
{
    return saveable.unsavedSince(levelPath, asItWouldBeSaved(levelData));
}

void LevelUi::update(
    const MouseOnTheMap &mouse,
    const Level &level,
    const LevelData &levelData,
    const std::string &levelPath,
    std::optional<Armed> &armed,
    EditorCommands &commands)
{
    if (saveable.lastSeen(levelPath).empty())
        saveable.seen(levelPath, asItWouldBeSaved(levelData));

    tileMapShown = whileArmed(tileMapShown, armed.has_value());

    if (!armed || mouse.overTheUi)
        return;

    const TileMap &tileMap = level.getTileMap();
    glm::ivec2 tilePosition = tileMap.tileContaining(mouse.worldPosition);
    if (!tileMap.validTilePosition(tilePosition))
        return;

    if (const PaintTile *painting = std::get_if<PaintTile>(&*armed))
    {
        if (!mouse.heldDown || tileMap.tilePositionToTileIndex(tilePosition) == painting->tileIndex)
            return;

        LevelData edited = levelData;
        edited.tileMapData = tileMap.toTileMapData();
        edited.tileMapData.indices[tilePosition.y][tilePosition.x] = painting->tileIndex;
        commands.onLevelEdited(edited);

        return;
    }

    if (!mouse.justClicked)
        return;

    PickTile picking = std::get<PickTile>(*armed);
    if (picking.what != PickTile::For::PlayerStart && picking.npcIndex >= levelData.npcs.size())
    {
        armed.reset();
        return;
    }

    LevelData edited = levelData;
    switch (picking.what)
    {
    case PickTile::For::PlayerStart:
        edited.playerFeet = tileMap.feetOnTile(tilePosition);
        break;

    case PickTile::For::NpcSpawn:
        edited.npcs[picking.npcIndex].feet = tileMap.feetOnTile(tilePosition);
        break;

    case PickTile::For::PatrolFrom:
    case PickTile::For::PatrolTo: {
        const std::optional<PatrolData> &walked = levelData.npcs[picking.npcIndex].patrol;
        std::pair<glm::ivec2, glm::ivec2> beat{tilePosition, tilePosition};
        if (walked)
            beat = tilesOfBeat(tileMap, *walked);

        if (picking.what == PickTile::For::PatrolFrom)
            beat.first = tilePosition;
        else
            beat.second = tilePosition;

        edited.npcs[picking.npcIndex].patrol = beatBetween(tileMap, beat.first, beat.second);
        break;
    }
    }

    commands.onLevelEdited(edited);
    armed.reset();
}

bool LevelUi::takesTheDisk(const LevelData &current, const std::string &levelPath)
{
    bool kept = unsavedSince(current, levelPath);
    std::optional<LevelData> onDisk = readLevelDataIfYouCan(levelPath);
    if (!onDisk)
        return !kept;

    std::string fromDisk = asItWouldBeSaved(*onDisk);
    bool changed = fromDisk != saveable.lastSeen(levelPath);
    saveable.saved(levelPath, fromDisk);

    return !kept && changed;
}
