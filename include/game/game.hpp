#pragma once

#include <memory>
#include <string_view>
#include "game/game_data.hpp"
#include "game/levels.hpp"
#include "tile_map/tile_interaction_system.hpp"
#include "actor/actor.hpp"
#include "player/player.hpp"
#include "npc/npc.hpp"
#include "game/scoring_system.hpp"
#include "rendering/shader.hpp"
#include "rendering/texture2d.hpp"
#include "rendering/sprite_renderer.hpp"
#include "rendering/tile_map_renderer.hpp"
#include "rendering/screen_transition.hpp"
#include "rendering/ui/editor_ui.hpp"
#include "rendering/ui/game_editor_ui.hpp"
#include "rendering/ui/imgui_manager.hpp"
#include "rendering/ui/debug_aabb_ui.hpp"
#include "rendering/ui/level_editor_ui.hpp"
#include "rendering/ui/navigation_ui.hpp"
#include "rendering/ui/npcs_ui.hpp"
#include "rendering/ui/score_ui.hpp"
#include "cameras/camera2d.hpp"
#include "input/keyboard_manager.hpp"
#include "input/input_manager.hpp"
#include "physics/fixed_time_step.hpp"
#include "scripting/lua_script_system.hpp"
#include "window/window.hpp"

class Game
{
public:
    explicit Game(Window &window);
    ~Game();
    void frame(float deltaTime);
    void pause();
    void step();
    void play();
    void loadLevel(const std::string &levelPath);
    void reload();
    void reloadShader(const std::string &shaderPath);
    void reloadTexture(const std::string &texturePath);
    void reloadScripts();
    bool isPlaying(const std::string &levelPath) const;
    void rebuildPlayer();
    void rebuildNpcs();
    void refreshActors();

private:
    std::unique_ptr<Shader> loadShader(std::string_view vertex, std::string_view fragment) const;
    void preFixedUpdate();
    void fixedUpdate(float deltaTime);
    void postFixedUpdate();
    void update(float deltaTime);
    void render();
    void resize(int width, int height);
    void rebuildLevel(const std::string &levelPath);

    Window &window;
    GameData gameData;
    Camera2D camera;
    KeyboardManager keyboardManager;
    InputManager inputManager;
    FixedTimeStep timestepper;
    LuaScriptSystem luaScriptSystem;
    std::unique_ptr<Level> level;
    std::unique_ptr<Player> player;
    std::vector<std::unique_ptr<Npc>> npcs;
    std::vector<Actor *> actors;
    TileInteractionSystem tileInteractionSystem;
    ScoringSystem scoringSystem;
    std::unique_ptr<Texture2D> tileSet, playerTexture;
    std::unique_ptr<Shader> tileSetShader, screenTransitionShader;
    SpriteRenderer spriteRenderer;
    TileMapRenderer tileMapRenderer;
    ScreenTransition screenTransition;
    ImGuiManager imGuiManager;
    Levels levels;
    EditorUi editorUi;
    GameEditorUi gameEditorUi;
    DebugAABBUi debugAABBUi;
    LevelEditorUi levelEditorUi;
    NavigationUi navigationUi;
    NpcsUi npcsUi;
    ScoreUi scoreUi;
    fteng::connection onLevelCompleteConnection;
    bool paused = false, stepFrame = false;
};