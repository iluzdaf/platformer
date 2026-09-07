#pragma once

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

class ImGuiManager;
class TileMap;
class Level;
class Camera2D;
struct NpcData;
struct Decided;
struct Observed;
struct ActorState;
class Npc;

struct EditorCommands;
struct Resize;

void askedToResize(
    Resize resize,
    const LevelData &levelData,
    int tileSize,
    EditorCommands &commands);

class LevelUi
{
public:
    void draw(
        const Level &level,
        const LevelData &levelData,
        const std::string &levelPath,
        const Decided &playerDecided,
        const Observed &playerObserved,
        const glm::vec2 &playerFeet,
        const ActorState &playerState,
        const std::map<std::string, NpcData> &npcData,
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

    void save(const LevelData &levelData, const std::string &levelPath);
    bool unsavedSince(const LevelData &levelData, const std::string &levelPath);
    bool followsTheDisk(const LevelData &current, const std::string &levelPath);

private:
    Saveable saveable;
    NavigationUi navigationUi;
    ActorShown showingActor;
    TileMapShown tileMapShown;

    std::string asItWouldBeSaved(const LevelData &levelData) const;
    void drawLevel(
        const Level &level,
        const LevelData &levelData,
        const std::string &levelPath,
        EditorCommands &commands);
    void drawActors(
        const Level &level,
        const LevelData &levelData,
        const Decided &playerDecided,
        const Observed &playerObserved,
        const glm::vec2 &playerFeet,
        const ActorState &playerState,
        const std::map<std::string, NpcData> &npcData,
        std::optional<Armed> &armed,
        EditorCommands &commands);
};