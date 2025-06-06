#pragma once
#include <memory>
#include "game/game_data.hpp"
#include "game/tile_map/tile_map.hpp"
#include "game/tile_map/tile_interaction_system.hpp"
#include "game/player/player.hpp"
#include "rendering/shader.hpp"
#include "rendering/tile_map_renderer.hpp"
#include "rendering/screen_transition.hpp"
#include "rendering/debug_renderer.hpp"
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
    void loadNextLevel();
    bool isPaused = false;

private:
    void initGameData();
    void initGLFW();
    void initGlad();
    void preFixedUpdate();
    void fixedUpdate(float deltaTime);
    void update(float deltaTime);
    void render();
    void resize(int width, int height);

    GLFWwindow *window;

    GameData gameData;

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
    std::unique_ptr<DebugRenderer> debugRenderer;

    fteng::connection onLevelCompleteConnection;
};