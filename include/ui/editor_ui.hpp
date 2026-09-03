#pragma once

#include "game/levels_data.hpp"

#include "game/level_data.hpp"

#include <array>
#include <functional>
#include <optional>
#include <string>
#include <glm/gtc/matrix_transform.hpp>
#include "ui/editor_commands.hpp"
#include "ui/editor_section.hpp"
#include "ui/camera_ui.hpp"
#include "ui/game_settings_ui.hpp"
#include "ui/types_ui.hpp"
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
class Npc;
class Player;
class TextureCache;

inline constexpr float InspectorWidth = 280.0f;

inline constexpr float SaveColumn = 120.0f;

struct SectionSaving
{
    bool unsaved = false;
    std::optional<std::string> cannotBecause;
    std::function<void()> save;
    std::function<void()> revert;
};

struct EditorSubject
{
    GameData &gameData;
    const Level &level;
    const LevelData &levelData;
    const std::string &levelPath;
    const TextureCache &textures;
    LevelsData &levels;
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
        const Level &level,
        const LevelData &levelData,
        const std::string &levelPath);

    EditorCommands commands;

    void valuesReplaced();

    SectionSaving savingIn(EditorSection listed, const EditorSubject &subject);

private:
    void drawSaveRow(const std::array<SectionSaving, EditorSections.size()> &saving);

private:
    EditorSection section = EditorSection::Playback;
    PlaybackUi playbackUi;
    GameSettingsUi gameSettingsUi;
    CameraUi cameraUi;
    PlayerUi playerUi;
    TypesUi typesUi;
    LevelUi levelUi;
    TilePalettesUi tilePalettesUi;
    LevelsUi levelsUi;
    std::optional<Armed> armed;
};
