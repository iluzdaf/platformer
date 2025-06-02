#pragma once
#include <memory>
#include "game/game_data.hpp"
#include "game/tile_map/tile_map.hpp"
#include "game/player/player.hpp"
#include "rendering/shader.hpp"
#include "rendering/tile_map_renderer.hpp"
#include "cameras/camera2d.hpp"
#include "input/keyboard_manager.hpp"
#include "physics/fixed_time_step.hpp"

class Game
{
public:
    Game();
    ~Game();
    void run();

private:
    void initGameData();
    void initGLFW();
    void initGlad();
    void preFixedUpdate();
    void fixedUpdate(float deltaTime);
    void update(float deltaTime);
    void render();
    void resize(int width, int height);
    void loadNextLevel();

    GLFWwindow *window;

    GameData gameData;

    std::unique_ptr<Camera2D> camera;
    KeyboardManager keyboardManager;
    FixedTimeStep timestepper;

    std::unique_ptr<TileMap> tileMap;
    std::unique_ptr<Player> player;

    std::unique_ptr<Texture2D> tileSet;
    Shader tileSetShader;
    std::unique_ptr<SpriteRenderer> tileSetSpriteRenderer;
    std::unique_ptr<TileMapRenderer> tileMapRenderer;
    std::unique_ptr<Texture2D> playerTexture;

    std::function<void()> onEndOfFrame;
};