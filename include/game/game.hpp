#pragma once

#include <memory>
#include "game/game_data.hpp"
#include "game/tile_map/tile_interaction_system.hpp"
#include "game/actor/actor.hpp"
#include "game/player/player.hpp"
#include "game/npc/npc.hpp"
#include "game/scoring_system.hpp"
#include "rendering/shader.hpp"
#include "rendering/texture2d.hpp"
#include "rendering/tile_map_renderer.hpp"
#include "rendering/screen_transition.hpp"
#include "rendering/ui/debug_ui.hpp"
#include "rendering/ui/imgui_manager.hpp"
#include "rendering/ui/debug_tile_map_ui.hpp"
#include "rendering/ui/debug_aabb_ui.hpp"
#include "rendering/ui/level_editor_ui.hpp"
#include "rendering/ui/score_ui.hpp"
#include "rendering/ui/debug_navigation_ui.hpp"
#include "cameras/camera2d.hpp"
#include "input/keyboard_manager.hpp"
#include "input/input_manager.hpp"
#include "physics/fixed_time_step.hpp"
#include "scripting/lua_script_system.hpp"
#include "reloading/level_watcher.hpp"
#include "reloading/asset_watcher.hpp"
#include "reloading/game_data_watcher.hpp"
#include "reloading/script_watcher.hpp"

class Game
{
public:
    Game();
    ~Game();
    void run();
    void pause();
    void step();
    void play();
    void loadLevel(const std::string &levelPath);
    void reload();
    void rebuildPlayer();
    void rebuildNpcs();
    void refreshActors();

private:
    GameData loadGameData() const;
    void initGLFW(int windowWidth, int windowHeight);
    void initGlad();
    void preFixedUpdate();
    void fixedUpdate(float deltaTime);
    void postFixedUpdate();
    void update(float deltaTime);
    void render();
    void resize(int width, int height);
    void rebuildLevel(const std::string &levelPath);

    GLFWwindow *window;
    std::unique_ptr<Camera2D> camera;
    KeyboardManager keyboardManager;
    InputManager inputManager;
    FixedTimeStep timestepper;
    std::unique_ptr<LuaScriptSystem> luaScriptSystem;
    LevelWatcher levelWatcher;
    AssetWatcher assetWatcher;
    GameDataWatcher gameDataWatcher;
    ScriptWatcher scriptWatcher;
    std::unique_ptr<Level> level;
    std::unique_ptr<Player> player;
    std::vector<std::unique_ptr<Npc>> npcs;
    std::vector<Actor *> actors;
    TileInteractionSystem tileInteractionSystem;
    ScoringSystem scoringSystem;
    std::unique_ptr<Texture2D> tileSet, playerTexture;
    std::unique_ptr<Shader> tileSetShader, screenTransitionShader;
    std::unique_ptr<SpriteRenderer> spriteRenderer;
    std::unique_ptr<TileMapRenderer> tileMapRenderer;
    std::unique_ptr<ScreenTransition> screenTransition;
    std::unique_ptr<ImGuiManager> imGuiManager;
    DebugUi debugUi;
    DebugTileMapUi debugTileMapUi;
    DebugAABBUi debugAABBUi;
    LevelEditorUi levelEditorUi;
    ScoreUi scoreUi;
    DebugNavigationUi debugNavigationUi;
    fteng::connection onLevelCompleteConnection;
    bool paused = false,
         stepFrame = false,
         shouldDrawGrid = false,
         shouldDrawTileInfo = false,
         shouldDrawPlayerAABBs = false,
         shouldDrawTileMapAABBs = false,
         showDebug = false,
         showLevelEditor = false;
    GameData gameData;
};