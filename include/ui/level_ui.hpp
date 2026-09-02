#pragma once

#include <string>
#include <cstddef>
#include <memory>
#include <map>
#include <optional>
#include <vector>
#include "ui/editor_commands.hpp"
#include "ui/grid_shown.hpp"
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
struct NpcData;
struct TilePalette;
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
        const Texture2D &sheet,
        const TilePalette &palette,
        const std::map<std::string, NpcData> &npcData,
        std::optional<Armed> &armed,
        EditorCommands &commands);
    void drawOverlay(const ImGuiManager &imGuiManager, const Camera2D &camera, const Level &level)
        const;
    void update(
        const MouseOnTheMap &mouse,
        Level &level,
        std::optional<Armed> &armed,
        EditorCommands &commands);

    void save(Level &level);
    bool hasUnsavedChanges(const Level &level);
    bool showingGrid() const;
    void valuesReplaced();

private:
    Saveable saveable;
    NavigationUi navigationUi;
    ActorShown showingActor;
    GridShown grid;
    bool drawTileInfo = false, drawTileColliders = false, drawLevelBounds = false;

    std::string asItWouldBeSaved(const Level &level) const;
    void drawLevel(Level &level);
    void drawActors(
        Level &level,
        const std::vector<std::unique_ptr<Npc>> &npcs,
        const ActorMotionState &playerMotionState,
        const glm::vec2 &playerFeet,
        const ActorState &playerState,
        const std::map<std::string, NpcData> &npcData,
        std::optional<Armed> &armed,
        EditorCommands &commands);
    void drawOverlayToggles();
    void drawTiles(const Texture2D &sheet, const TilePalette &palette, std::optional<Armed> &armed);
};