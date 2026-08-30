#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "rendering/ui/editor_commands.hpp"
#include "rendering/ui/editor_section.hpp"
#include "rendering/ui/camera_ui.hpp"
#include "rendering/ui/debug_aabb_overlay.hpp"
#include "rendering/ui/game_settings_ui.hpp"
#include "rendering/ui/npc_types_ui.hpp"
#include "rendering/ui/playback_ui.hpp"
#include "rendering/ui/player_ui.hpp"
#include "rendering/ui/brush.hpp"
#include "rendering/ui/level_ui.hpp"
#include "rendering/ui/levels_ui.hpp"
#include "rendering/ui/navigation_ui.hpp"
#include "rendering/ui/npcs_ui.hpp"
#include "rendering/ui/tile_palettes_ui.hpp"

struct ActorMotionState;
struct ActorState;
struct GameData;
class Camera2D;
class ImGuiManager;
class Level;
class Levels;
class Npc;
class Player;
class Texture2D;

struct EditorSubject
{
    GameData &gameData;
    Level &level;
    const std::vector<std::unique_ptr<Npc>> &npcs;
    const Texture2D &tileSet;
    Levels &levels;
    const ActorMotionState &playerMotionState;
    glm::vec2 playerPosition;
    const ActorState &playerState;
    const Camera2D &camera;
    bool paused = false;
};

class EditorUi
{
public:
    void draw(const ImGuiManager &imGuiManager, const EditorSubject &subject, bool showEditors);
    void drawOverlays(
        const ImGuiManager &imGuiManager,
        const Camera2D &camera,
        const Level &level,
        const Player &player);
    void update(
        float deltaTime,
        const ImGuiManager &imGuiManager,
        const Camera2D &camera,
        Level &level);

    EditorCommands commands;

    void valuesReplaced();

private:
    EditorSection section = EditorSection::Playback;
    DebugAABBOverlay debugAABBOverlay;
    PlaybackUi playbackUi;
    GameSettingsUi gameSettingsUi;
    CameraUi cameraUi;
    PlayerUi playerUi;
    NpcTypesUi npcTypesUi;
    LevelUi levelUi;
    NavigationUi navigationUi;
    NpcsUi npcsUi;
    TilePalettesUi tilePalettesUi;
    LevelsUi levelsUi;
    std::optional<Brush> brush;
};
