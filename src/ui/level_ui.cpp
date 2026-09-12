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
#include "ui/editor_history.hpp"
#include "ui/saveable.hpp"
#include "ui/saved_in_scope.hpp"
#include "ui/data_inspector.hpp"
#include "animations/animator_data.hpp"
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
#include "npc/npc_data.hpp"
#include "pickups/pickup_data.hpp"
#include "pickups/pickup_spawn_data.hpp"
#include "ui/tile_map_overlays.hpp"
#include <cfloat>
#include "ui/inspector_edited.hpp"
#include "ui/marked_label.hpp"
#include "tile_map/tile_palette_data.hpp"
#include "ui/picking_in_level.hpp"
#include "ui/size_buttons.hpp"
#include <exception>
#include "game/game_data.hpp"
#include "game/level_resizing.hpp"
#include "cameras/camera2d.hpp"

LevelUi::LevelUi(EditorHistory &history) : history(history)
{
}

void LevelUi::draw(
    const Level &level,
    const LevelData &levelData,
    const std::optional<AnimatorData> &playerAnimations,
    const Observed &playerObserved,
    const glm::vec2 &playerFeet,
    const Appearance &playerAppearance,
    const std::map<std::string, NpcData> &npcData,
    std::optional<Armed> &armed,
    EditorCommands &commands)
{
    drawLevel(level, levelData, commands);
    navigationUi.draw(level);
    drawActors(
        level,
        levelData,
        playerAnimations,
        playerObserved,
        playerFeet,
        playerAppearance,
        npcData,
        armed,
        commands);
}

void LevelUi::drawActors(
    const Level &level,
    const LevelData &levelData,
    const std::optional<AnimatorData> &playerAnimations,
    const Observed &playerObserved,
    const glm::vec2 &playerFeet,
    const Appearance &playerAppearance,
    const std::map<std::string, NpcData> &npcData,
    std::optional<Armed> &armed,
    EditorCommands &commands)
{
    ActorShown wasShowing = showingActor;
    ActorAsked asked = drawActorsInLevel(
        level,
        playerAnimations,
        playerObserved,
        playerFeet,
        playerAppearance,
        npcData,
        showingActor,
        armed);

    const bool deleted = ImGui::IsKeyPressed(ImGuiKey_Delete) && !ImGui::IsAnyItemActive();
    asked.removeShown = asked.removeShown || deleted;

    if (asked.removeShown && showingActor.what == ActorShown::What::Npc)
    {
        LevelData edited = levelData;
        edited.npcs.erase(edited.npcs.begin() + static_cast<std::ptrdiff_t>(showingActor.index));

        showingActor = ActorShown{};
        history.remembers(levelData);
        commands.onLevelEdited(edited);
    }
    else if (asked.removeShown && showingActor.what == ActorShown::What::Pickup)
    {
        LevelData edited = levelData;
        edited.pickups.erase(
            edited.pickups.begin() + static_cast<std::ptrdiff_t>(showingActor.index));

        showingActor = ActorShown{};
        history.remembers(levelData);
        commands.onLevelEdited(edited);
    }
    else if (asked.clearShownBeat && showingActor.what == ActorShown::What::Npc)
    {
        LevelData edited = levelData;
        edited.npcs[showingActor.index].patrol.reset();
        history.remembers(levelData);
        commands.onLevelEdited(edited);
    }
    else
        showingActor = asked.show;

    if (showingActor != wasShowing)
        armed.reset();
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

namespace
{
    constexpr float ButtonsWidth = 124.0f;

    inspector::Edited drawPaletteNamed(std::string &shown, const TilePalettes &tilePalettes)
    {
        inspector::InField here("tilePalette");
        const bool changed = inspector::changedHere(shown);
        const bool noSuchPalette = !tilePalettes.contains(shown);
        inspector::Marking marking(changed);
        inspector::drawLabel("tilePalette", changed, noSuchPalette);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-ButtonsWidth);

        inspector::Edited edited;
        if (ImGui::BeginCombo("##tilePalette", shown.c_str()))
        {
            for (const auto &[name, palette] : tilePalettes)
                if (ImGui::Selectable(name.c_str(), name == shown) && name != shown)
                {
                    shown = name;
                    edited = {true, true};
                }

            ImGui::EndCombo();
        }

        if (noSuchPalette)
            inspector::drawRefusal("no palette called " + shown);

        return edited;
    }
}

PaletteAsked LevelUi::drawTilePaletteNamed(
    const LevelData &levelData,
    const TilePalettes &tilePalettes,
    EditorCommands &commands)
{
    SavedInScope was(asItWasSaved<LevelData>(saveable.lastSeen(editing)));

    LevelData edited = levelData;
    inspector::Edited picked;
    {
        inspector::InField tileMapData("tileMapData");
        picked = drawPaletteNamed(edited.tileMapData.tilePalette, tilePalettes);
    }

    if (picked)
    {
        history.remembers(levelData);
        commands.onLevelEdited(edited);
    }

    PaletteAsked asked;
    ImGui::SameLine();
    asked.add = ImGui::Button("add");
    ImGui::SameLine();
    ImGui::BeginDisabled(tilePalettes.empty());
    asked.remove = ImGui::Button("remove", ImVec2(-FLT_MIN, 0.0f));
    ImGui::EndDisabled();

    return asked;
}

void LevelUi::namesPalette(
    const std::string &palette,
    const LevelData &levelData,
    EditorCommands &commands)
{
    if (levelData.tileMapData.tilePalette == palette)
        return;

    LevelData edited = levelData;
    edited.tileMapData.tilePalette = palette;
    history.remembers(levelData);
    commands.onLevelEdited(edited);
}

void LevelUi::drawLevel(const Level &level, const LevelData &levelData, EditorCommands &commands)
{
    SavedInScope was(asItWasSaved<LevelData>(saveable.lastSeen(editing)));

    LevelData edited = levelData;
    if (inspector::draw("nextLevel", edited.nextLevel))
    {
        history.remembers(levelData);
        commands.onLevelEdited(edited);
    }

    if (!ImGui::TreeNodeEx("Resize"))
        return;
    const TileMap &tileMap = level.getTileMap();
    if (std::optional<Resize> resize = drawSizeButtons(tileMap.getWidth(), tileMap.getHeight()))
        resizes(*resize, levelData, tileMap.getTileSize(), commands);
    ImGui::TreePop();
}

ActorShown LevelUi::shown() const
{
    return showingActor;
}

void LevelUi::resizes(
    Resize resize,
    const LevelData &levelData,
    int tileSize,
    EditorCommands &commands)
{
    history.remembers(levelData, -shiftOf(resize, tileSize));
    askedToResize(resize, levelData, tileSize, commands);
}

void LevelUi::forgets()
{
    history.forgets();
}

void LevelUi::aNewLevel(const std::string &levelPath)
{
    saveable.neverSaved(levelPath);
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
    const AABB &playerBox,
    const std::map<std::string, NpcData> &npcData,
    const std::map<std::string, PickupData> &pickupData,
    std::optional<Armed> &armed,
    EditorCommands &commands)
{
    if (saveable.lastSeen(levelPath).empty())
        saveable.seen(levelPath, asItWouldBeSaved(levelData));

    if (levelPath != editing)
    {
        history.forgets();
        editing = levelPath;
    }

    tileMapShown = whileArmed(tileMapShown, armed.has_value());

    if (!mouse.heldDown)
        paintingAStroke = false;

    if (mouse.overTheUi)
        return;

    if (!armed)
    {
        if (mouse.justClicked)
            showingActor = whatIsAt(level, playerBox, mouse.worldPosition);

        return;
    }

    const TileMap &tileMap = level.getTileMap();
    glm::ivec2 tilePosition = tileMap.tileContaining(mouse.worldPosition);
    if (!tileMap.validTilePosition(tilePosition))
        return;

    if (const PlaceOne *placing = beingPlaced(armed))
    {
        if (!mouse.justClicked)
            return;

        LevelData edited = levelData;
        glm::vec2 feet = tileMap.feetOnTile(tilePosition);
        if (placing->what == PlaceOne::What::Npc)
        {
            auto kind = npcData.find(placing->type);
            if (kind == npcData.end())
            {
                armed.reset();
                return;
            }

            NpcSpawnData spawn{placing->type, feet, std::nullopt};
            spawn.patrol = level.runBeneath(buildNavigationProfile(kind->second.actorData), feet);
            edited.npcs.push_back(spawn);
            showingActor = ActorShown{ActorShown::What::Npc, edited.npcs.size() - 1};
        }
        else
        {
            if (!pickupData.contains(placing->type))
            {
                armed.reset();
                return;
            }

            edited.pickups.push_back(PickupSpawnData{placing->type, feet});
            showingActor = ActorShown{ActorShown::What::Pickup, edited.pickups.size() - 1};
        }

        history.remembers(levelData);
        commands.onLevelEdited(edited);

        return;
    }

    if (const PaintTile *painting = std::get_if<PaintTile>(&*armed))
    {
        if (!mouse.heldDown || tileMap.tilePositionToTileIndex(tilePosition) == painting->tileIndex)
            return;

        LevelData edited = levelData;
        edited.tileMapData = tileMap.toTileMapData();
        if (!paintingAStroke)
        {
            history.remembers(edited);
            paintingAStroke = true;
        }

        edited.tileMapData.indices[tilePosition.y][tilePosition.x] = painting->tileIndex;
        commands.onTilesChanged(edited.tileMapData);

        return;
    }

    if (!mouse.justClicked)
        return;

    PickTile picking = std::get<PickTile>(*armed);
    const std::size_t among = picking.what == PickTile::For::PickupSpawn ? levelData.pickups.size()
                                                                         : levelData.npcs.size();
    if (picking.what != PickTile::For::PlayerStart && picking.index >= among)
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
        edited.npcs[picking.index].feet = tileMap.feetOnTile(tilePosition);
        break;

    case PickTile::For::PickupSpawn:
        edited.pickups[picking.index].feet = tileMap.feetOnTile(tilePosition);
        break;

    case PickTile::For::PatrolFrom:
    case PickTile::For::PatrolTo: {
        const std::optional<PatrolData> &walked = levelData.npcs[picking.index].patrol;
        std::pair<glm::ivec2, glm::ivec2> beat{tilePosition, tilePosition};
        if (walked)
            beat = tilesOfBeat(tileMap, *walked);

        if (picking.what == PickTile::For::PatrolFrom)
            beat.first = tilePosition;
        else
            beat.second = tilePosition;

        edited.npcs[picking.index].patrol = beatBetween(tileMap, beat.first, beat.second);
        break;
    }
    }

    history.remembers(levelData);
    commands.onLevelEdited(edited);
    armed.reset();
}

std::optional<std::string> LevelUi::cannotSaveBecause(
    const LevelData &levelData,
    const GameData &gameData)
{
    return walkGate.to(
        asItWouldBeSaved(levelData) + asJson(gameData),
        [&]() -> std::optional<std::string>
        {
            try
            {
                Level built(
                    levelData,
                    gameData.tilePalettes,
                    gameData.playerData,
                    gameData.npcData,
                    gameData.pickupData);

                return npcsThatCannotGetBack(built);
            }
            catch (const std::exception &refused)
            {
                return refused.what();
            }
        });
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

    if (!kept && changed)
    {
        history.forgets();
        return true;
    }

    return false;
}
