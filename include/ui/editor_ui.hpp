#pragma once

#include <memory>
#include <optional>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "ui/editor_commands.hpp"
#include "ui/editor_section.hpp"
#include "ui/camera_ui.hpp"
#include "ui/game_settings_ui.hpp"
#include "ui/npc_types_ui.hpp"
#include "ui/playback_ui.hpp"
#include "ui/player_ui.hpp"
#include "ui/armed.hpp"
#include "ui/level_ui.hpp"
#include "ui/levels_ui.hpp"
#include "ui/tile_palettes_ui.hpp"

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

inline constexpr float InspectorWidth = 280.0f;

struct EditorSubject
{
    GameData &gameData;
    Level &level;
    const std::vector<std::unique_ptr<Npc>> &npcs;
    const Texture2D &tileSet;
    Levels &levels;
    const ActorMotionState &playerMotionState;
    glm::vec2 playerFeet;
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
    bool unsavedIn(EditorSection listed, const EditorSubject &subject) const;

private:
    EditorSection section = EditorSection::Playback;
    PlaybackUi playbackUi;
    GameSettingsUi gameSettingsUi;
    CameraUi cameraUi;
    PlayerUi playerUi;
    NpcTypesUi npcTypesUi;
    LevelUi levelUi;
    TilePalettesUi tilePalettesUi;
    LevelsUi levelsUi;
    std::optional<Armed> armed;
};
