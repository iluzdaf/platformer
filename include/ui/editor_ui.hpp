#pragma once

#include "game/levels_data.hpp"

#include "game/level_data.hpp"

#include <array>
#include <functional>
#include <optional>
#include <string>
#include <glm/gtc/matrix_transform.hpp>
#include "ui/editor_commands.hpp"
#include "ui/editor_history.hpp"
#include "ui/edit_settling.hpp"
#include "ui/editor_section.hpp"
#include "ui/camera_ui.hpp"
#include "ui/game_settings_ui.hpp"
#include "ui/types_ui.hpp"
#include "ui/playback_ui.hpp"
#include "ui/player_overlay_ui.hpp"
#include "ui/armed.hpp"
#include "ui/level_ui.hpp"
#include "ui/levels_ui.hpp"
#include "ui/tile_palettes_ui.hpp"

struct AbilityStates;
struct Observed;
struct Appearance;
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
    const AbilityStates &playerAbilityStates;
    const Observed &playerObserved;
    glm::vec2 playerFeet;
    const Appearance &playerAppearance;
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
        const std::string &levelPath,
        const AABB &playerBox,
        const GameData &gameData);

    EditorCommands commands;

    struct Reloaded
    {
        bool cast = false;
        bool palettes = false;
    };

    Reloaded reloaded(GameData &current, const GameData &onDisk);
    bool levelTakesTheDisk(const LevelData &current, const std::string &levelPath);

    void show(EditorSection listed);
    EditorSection shown() const;
    void remembersWhatChanged(const EditorSubject &subject, bool stillBeingEdited);
    void remembers(EditorStep step);
    bool undo(const EditorSubject &subject);
    bool anythingToUndo() const;

    SectionSaving savingIn(EditorSection listed, const EditorSubject &subject);

private:
    void drawProblems(const std::array<SectionSaving, EditorSections.size()> &saving);
    void drawSaveRow(const std::array<SectionSaving, EditorSections.size()> &saving);
    void drawSectionTabs(const std::array<SectionSaving, EditorSections.size()> &saving);
    void drawUndoRow(const EditorSubject &subject);
    void forgetsIfNamesChanged(const EditorSubject &subject);
    void putsBack(const std::string &gameDataAsItWas, GameData &gameData);
    void putsBack(
        const LevelData &asItWas,
        const glm::vec2 &movingThePlayerBack,
        const LevelData &now);

private:
    EditorHistory history;
    EditSettling editing;
    GameSettingsUi gameSettingsUi;
    CameraUi cameraUi;
    PlayerOverlayUi playerOverlayUi;
    TypesUi typesUi;
    LevelUi levelUi{history};
    TilePalettesUi tilePalettesUi;
    LevelsUi levelsUi;
    std::optional<Armed> armed;
    EditorSection section = EditorSection::Runtime;
    float panelWidth = InspectorWidth;
    PlaybackUi playbackUi;
    bool askedToShow = false;
};
