#pragma once
#include <memory>
#include "game/game_data.hpp"
#include "game/tile_map/tile_map.hpp"
#include "game/tile_map/tile_interaction_system.hpp"
#include "game/player/player.hpp"
#include "rendering/shader.hpp"
#include "rendering/tile_map_renderer.hpp"
#include "rendering/screen_transition.hpp"
#include "rendering/ui/debug_ui.hpp"
#include "rendering/ui/imgui_manager.hpp"
#include "rendering/ui/debug_tile_map_ui.hpp"
#include "rendering/ui/debug_aabb_ui.hpp"
#include "rendering/ui/editor_tile_map_ui.hpp"
#include "cameras/camera2d.hpp"
#include "input/keyboard_manager.hpp"
#include "physics/fixed_time_step.hpp"
#include "scripting/lua_script_system.hpp"

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

    std::unique_ptr<TileMap> tileMap;
    std::unique_ptr<Player> player;
    std::unique_ptr<TileInteractionSystem> tileInteractionSystem;

    std::unique_ptr<Texture2D> tileSet;
    Shader tileSetShader;
    std::unique_ptr<SpriteRenderer> tileSetSpriteRenderer;
    std::unique_ptr<TileMapRenderer> tileMapRenderer;
    std::unique_ptr<Texture2D> playerTexture;
    std::unique_ptr<ScreenTransition> screenTransition;
    Shader screenTransitionShader;
    std::unique_ptr<DebugUi> debugControlUi;
    std::unique_ptr<ImGuiManager> imGuiManager;
    std::unique_ptr<DebugTileMapUi> debugTileMapUi;
    std::unique_ptr<DebugAABBUi> debugAABBUi;
    std::unique_ptr<EditorTileMapUi> editorTileMapUi;

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