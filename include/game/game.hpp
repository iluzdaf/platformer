#pragma once
#include <memory>
#include "game/game_data.hpp"
#include "game/tile_map/tile_map.hpp"
#include "game/tile_map/tile_interaction_system.hpp"
#include "game/player/player.hpp"
#include "game/scoring_system.hpp"
#include "rendering/shader.hpp"
#include "rendering/texture2d.hpp"
#include "rendering/tile_map_renderer.hpp"
#include "rendering/screen_transition.hpp"
#include "rendering/ui/debug_ui.hpp"
#include "rendering/ui/imgui_manager.hpp"
#include "rendering/ui/debug_tile_map_ui.hpp"
#include "rendering/ui/debug_aabb_ui.hpp"
#include "rendering/ui/editor_tile_map_ui.hpp"
#include "rendering/ui/score_ui.hpp"
#include "cameras/camera2d.hpp"
#include "input/keyboard_manager.hpp"
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

private:
    GameData loadGameData() const;
    void initGLFW(int windowWidth, int windowHeight);
    void initGlad();
    void preFixedUpdate();
    void fixedUpdate(float deltaTime);
    void update(float deltaTime);
    void render();
    void resize(int width, int height);

    GLFWwindow *window;

    std::unique_ptr<Camera2D> camera;
    KeyboardManager keyboardManager;
    FixedTimeStep timestepper;
    std::unique_ptr<LuaScriptSystem> luaScriptSystem;
    LevelWatcher levelWatcher;
    AssetWatcher assetWatcher;
    GameDataWatcher gameDataWatcher;
    ScriptWatcher scriptWatcher;

    std::unique_ptr<TileMap> tileMap;
    std::unique_ptr<Player> player;
    TileInteractionSystem tileInteractionSystem;
    ScoringSystem scoringSystem;

    std::unique_ptr<Texture2D> tileSet;
    std::unique_ptr<Shader> tileSetShader;
    std::unique_ptr<SpriteRenderer> spriteRenderer;
    std::unique_ptr<TileMapRenderer> tileMapRenderer;
    std::unique_ptr<Texture2D> playerTexture;
    std::unique_ptr<ScreenTransition> screenTransition;
    std::unique_ptr<Shader> screenTransitionShader;
    std::unique_ptr<ImGuiManager> imGuiManager;
    DebugUi debugUi;
    DebugTileMapUi debugTileMapUi;
    DebugAABBUi debugAABBUi;
    EditorTileMapUi editorTileMapUi;
    ScoreUi scoreUi;

    fteng::connection onLevelCompleteConnection;

    bool paused = false,
         stepFrame = false,
         shouldDrawGrid = false,
         shouldDrawTileInfo = false,
         shouldDrawPlayerAABBs = false,
         shouldDrawTileMapAABBs = false,
         showDebug = false,
         showTileMapEditor = false;
};