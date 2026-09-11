#pragma once

#include "npc/npc_data.hpp"

#include "ui/last_answer.hpp"

#include "ui/editor_history.hpp"

#include "game/level_resizing.hpp"

#include "game/level_data.hpp"

#include <string>
#include <cstddef>
#include <map>
#include <optional>
#include "ui/editor_commands.hpp"
#include "ui/tile_map_shown.hpp"
#include "ui/armed.hpp"
#include "ui/mouse_on_the_map.hpp"
#include "ui/actors_in_level.hpp"
#include "ui/navigation_ui.hpp"
#include "ui/saveable.hpp"
#include "tile_map/tile_palette_data.hpp"

class ImGuiManager;
class TileMap;
class Level;
class Camera2D;
struct NpcData;
struct PickupData;
struct Decided;
struct ActorAnimationData;
struct Observed;
struct ActorState;
class Npc;

struct EditorCommands;

void askedToResize(
    Resize resize,
    const LevelData &levelData,
    int tileSize,
    EditorCommands &commands);

class LevelUi
{
public:
    explicit LevelUi(EditorHistory &history);

    void drawTilePaletteNamed(
        const LevelData &levelData,
        const TilePalettes &tilePalettes,
        EditorCommands &commands);
    void draw(
        const Level &level,
        const LevelData &levelData,
        const ActorAnimationData &playerAnimations,
        const Observed &playerObserved,
        const glm::vec2 &playerFeet,
        const ActorState &playerState,
        const std::map<std::string, NpcData> &npcData,
        const std::map<std::string, PickupData> &pickupData,
        std::optional<Armed> &armed,
        EditorCommands &commands);
    void drawOverlay(const ImGuiManager &imGuiManager, const Camera2D &camera, const Level &level)
        const;
    void drawOverlayToggles();
    void update(
        const MouseOnTheMap &mouse,
        const Level &level,
        const LevelData &levelData,
        const std::string &levelPath,
        std::optional<Armed> &armed,
        EditorCommands &commands);

    void resizes(Resize resize, const LevelData &levelData, int tileSize, EditorCommands &commands);
    bool undo(EditorCommands &commands);
    void forgets();
    void aNewLevel(const std::string &levelPath);

    void save(const LevelData &levelData, const std::string &levelPath);
    bool unsavedSince(const LevelData &levelData, const std::string &levelPath);
    std::optional<std::string> cannotSaveBecause(
        const Level &level,
        const LevelData &levelData,
        const std::map<std::string, NpcData> &npcs);
    bool takesTheDisk(const LevelData &current, const std::string &levelPath);

private:
    Saveable saveable;
    LastAnswer walkGate;
    EditorHistory &history;
    std::string editing;
    bool paintingAStroke = false;
    NavigationUi navigationUi;
    ActorShown showingActor;
    TileMapShown tileMapShown;
    bool npcsShown = false;

    std::string asItWouldBeSaved(const LevelData &levelData) const;
    void drawLevel(const Level &level, const LevelData &levelData, EditorCommands &commands);
    void drawActors(
        const Level &level,
        const LevelData &levelData,
        const ActorAnimationData &playerAnimations,
        const Observed &playerObserved,
        const glm::vec2 &playerFeet,
        const ActorState &playerState,
        const std::map<std::string, NpcData> &npcData,
        const std::map<std::string, PickupData> &pickupData,
        std::optional<Armed> &armed,
        EditorCommands &commands);
};