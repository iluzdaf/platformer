#pragma once

#include <cstddef>
#include <memory>
#include <optional>
#include <vector>
#include "ui/editor_commands.hpp"
#include "ui/armed.hpp"
#include "ui/mouse_on_the_map.hpp"
#include "ui/actors_in_level.hpp"
#include "ui/navigation_ui.hpp"
#include "ui/saveable.hpp"

class ImGuiManager;
class TileMap;
class Level;
class Texture2D;
class Camera2D;
struct GameData;
struct ActorMotionState;
struct ActorState;
class Npc;

class LevelUi
{
public:
    void draw(
        Level &level,
        const std::vector<std::unique_ptr<Npc>> &npcs,
        const ActorMotionState &playerMotionState,
        const glm::vec2 &playerFeet,
        const ActorState &playerState,
        const Texture2D &tileSet,
        const GameData &gameData,
        std::optional<Armed> &armed,
        EditorCommands &commands);
    void drawOverlay(const ImGuiManager &imGuiManager, const Camera2D &camera, const Level &level)
        const;
    void update(
        const MouseOnTheMap &mouse,
        Level &level,
        std::optional<Armed> &armed,
        EditorCommands &commands);

    bool hasUnsavedChanges(const Level &level) const;
    void valuesReplaced();

private:
    Saveable saveable;
    NavigationUi navigationUi;
    ActorShown showingActor;
    bool drawGrid = false, drawTileInfo = false, drawTileColliders = false, drawLevelBounds = false;

    void drawLevel(Level &level, EditorCommands &commands);
    void drawOverlayToggles();
    void drawTiles(
        Level &level,
        const Texture2D &tileSet,
        const GameData &gameData,
        std::optional<Armed> &armed);
};