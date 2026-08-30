#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include "game/game.hpp"
#include "game/game_data.hpp"
#include "game/level.hpp"
#include "game/levels.hpp"
#include "game/world.hpp"
#include "player/player.hpp"
#include "ui/game_ui.hpp"
#include "rendering/screen_transition.hpp"
#include "cameras/camera2d.hpp"
#include "assets/asset_paths.hpp"
#include "window/window.hpp"
#include "scripting/lua_script_system.hpp"
#include "reloading/reload_commands.hpp"

Game::Game(Window &window, ReloadCommands &reloadCommands)
    : window(window), gameData(loadGameData()),
      camera(gameData.cameraData, window.getFramebufferSize().x, window.getFramebufferSize().y),
      world(gameData, keyboardIntentions, luaScriptSystem),
      gameUi(window, window.getFramebufferSize().x, window.getFramebufferSize().y),
      levels(assets::pathTo(assets::LevelList))
{
    window.setSize(gameData.settings.windowWidth, gameData.settings.windowHeight);
    onResizeConnection = window.onResize.connect(
        [this](int width, int height)
        {
            camera.resize(width, height);
            gameUi.resize(width, height);
        });

    keyboardManager.registerKey(GLFW_KEY_P);
    keyboardManager.registerKey(GLFW_KEY_S);
    keyboardManager.registerKey(GLFW_KEY_F1);
    showEditors = gameData.settings.debug;

    world.onLevelLoaded.connect(
        [this]
        {
            const TileMap &tileMap = world.getLevel().getTileMap();
            camera.setWorldBounds(
                glm::vec2(0), glm::vec2(tileMap.getWorldWidth(), tileMap.getWorldHeight()));
        });
    world.loadLevel(levels.getFirst());

    gameUi.commands().onPlay.connect([this] { playback.play(); });
    gameUi.commands().onPause.connect([this] { playback.pause(); });
    gameUi.commands().onStep.connect([this] { playback.step(); });
    gameUi.commands().onLoadLevel.connect([this](const std::string &levelPath)
                                          { world.loadLevel(levelPath); });
    gameUi.commands().onRespawn.connect([this] { world.respawnPlayer(); });
    gameUi.commands().onSettingsChanged.connect(
        [this]
        { this->window.setSize(gameData.settings.windowWidth, gameData.settings.windowHeight); });
    gameUi.commands().onCameraChanged.connect([this] { camera.setZoom(gameData.cameraData.zoom); });

    reloadCommands.isPlaying = [this](const std::string &levelPath)
    { return world.isPlaying(levelPath); };
    reloadConnections.push_back(reloadCommands.onLoadLevel.connect(
        [this](const std::string &levelPath) { world.loadLevel(levelPath); }));
    reloadConnections.push_back(reloadCommands.onReloadShader.connect(
        [this](const std::string &shaderPath) { renderer.reloadShader(shaderPath); }));
    reloadConnections.push_back(reloadCommands.onReloadTexture.connect(
        [this](const std::string &texturePath) { renderer.reloadTexture(texturePath); }));
    reloadConnections.push_back(reloadCommands.onReload.connect(
        [this]
        {
            gameData = loadGameData();
            gameUi.valuesReplaced();
            this->window.setSize(gameData.settings.windowWidth, gameData.settings.windowHeight);
            camera.setZoom(gameData.cameraData.zoom);

            std::string current = world.getLevelPath();
            world.loadLevel(current.empty() ? levels.getFirst() : current);
        }));
    reloadConnections.push_back(
        reloadCommands.onReloadScripts.connect([this] { luaScriptSystem.loadScripts(); }));

    luaScriptSystem.bindGameObjects(&playback, &camera, &screenTransition, &world);

    luaScriptSystem.triggerGameLoaded();
}

Game::~Game() = default;

void Game::frame(float deltaTime)
{
    keyboardManager.poll(window.getHandle());
    if (keyboardManager.isPressed(GLFW_KEY_P))
        playback.isPaused() ? playback.play() : playback.pause();
    if (keyboardManager.isPressed(GLFW_KEY_F1))
        showEditors = !showEditors;
    if (keyboardManager.isPressed(GLFW_KEY_S))
        playback.step();

    luaScriptSystem.update(deltaTime);
    camera.update(deltaTime);
    screenTransition.update(deltaTime);
    gameUi.update(deltaTime, world.getLevel(), camera);

    playback.advance(
        deltaTime,
        [this]
        {
            keyboardIntentions.process(window.getHandle());
            world.preFixedUpdate();
        },
        [this](float dt)
        {
            world.fixedUpdate(dt);
            world.postFixedUpdate();
        },
        [this](float dt) { world.update(dt); });

    camera.follow(world.getPlayer().getPosition());

    render();
}

void Game::render()
{
    renderer.draw(camera.getProjection(), world.getLevel().getTileMap(), world.getActors());

    gameUi.draw(
        GameUiSubject{
            gameData,
            world.getLevel(),
            world.getNpcs(),
            world.getPlayer(),
            renderer.getTileSet(),
            levels,
            camera,
            world.getScoringSystem(),
            playback.isPaused(),
            showEditors});

    renderer.draw(screenTransition);
}
