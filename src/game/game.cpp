#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include "game/game.hpp"
#include "game/reloads.hpp"
#include "game/game_data.hpp"
#include "game/level.hpp"
#include "game/world.hpp"
#include "game/level_data.hpp"
#include "player/player.hpp"
#include "ui/game_ui.hpp"
#include "rendering/screen_transition.hpp"
#include "cameras/camera2d.hpp"
#include "window/window.hpp"
#include "input/keys_down.hpp"
#include "input/keys_unless_captured.hpp"
#include "scripting/lua_script_system.hpp"
#include "reloading/reloader.hpp"

Game::Game(Window &window, Reloader &reloader)
    : window(window), gameData(loadGameData()),
      camera(gameData.cameraData, window.getFramebufferSize().x, window.getFramebufferSize().y),
      world(gameData, keyboardIntentions, luaScriptSystem),
      gameUi(window, window.getFramebufferSize().x, window.getFramebufferSize().y)
{
    window.setSize(gameData.settings.windowWidth, gameData.settings.windowHeight);
    onResizeConnection = window.onResize.connect(
        [this](int width, int height)
        {
            camera.resize(width, height);
            gameUi.resize(width, height);
        });

    keys.registerKey(GLFW_KEY_P);
    keys.registerKey(GLFW_KEY_S);
    keys.registerKey(GLFW_KEY_F1);
    showEditors = gameData.settings.debug;

    world.onLevelBuilt.connect(
        [this]
        {
            const TileMap &tileMap = world.getLevel().getTileMap();
            camera.setWorldBounds(
                glm::vec2(0), glm::vec2(tileMap.getWorldWidth(), tileMap.getWorldHeight()));
        });
    world.onLevelBuilt.connect([this, &reloader] { reloader.levelLoaded(world.getLevelPath()); });
    renderer.warm(gameData);
    world.loadLevel(gameData.levels.first);

    gameUi.commands().onPlay.connect([this] { playback.play(); });
    gameUi.commands().onPause.connect([this] { playback.pause(); });
    gameUi.commands().onStep.connect([this] { playback.step(); });
    gameUi.commands().onLoadLevel.connect([this](const std::string &levelPath)
                                          { world.loadLevel(levelPath); });
    gameUi.commands().onRespawn.connect([this] { world.respawnPlayer(); });
    gameUi.commands().onLevelEdited.connect([this](const LevelData &edited)
                                            { world.rebuildFrom(edited); });
    gameUi.commands().onSettingsChanged.connect(
        [this]
        { this->window.setSize(gameData.settings.windowWidth, gameData.settings.windowHeight); });
    gameUi.commands().onCameraChanged.connect([this] { camera.setZoom(gameData.cameraData.zoom); });
    gameUi.commands().onWarmTexture.connect([this](const std::string &texturePath)
                                            { renderer.warmTexture(texturePath); });

    reloadConnections.push_back(reloader.commands.onLoadLevel.connect(
        [this](const std::string &levelPath)
        { reloads::levelChanged(world, gameUi.editor(), levelPath); }));
    reloadConnections.push_back(reloader.commands.onReloadShader.connect(
        [this](const std::string &shaderPath) { renderer.reloadShader(shaderPath); }));
    reloadConnections.push_back(reloader.commands.onReloadTexture.connect(
        [this](const std::string &texturePath) { renderer.reloadTexture(texturePath); }));
    reloadConnections.push_back(reloader.commands.onReload.connect(
        [this]
        {
            GameData onDisk = loadGameData();
            reloads::gameDataChanged(world, gameUi.editor(), gameData, onDisk);
            this->window.setSize(gameData.settings.windowWidth, gameData.settings.windowHeight);
            camera.setZoom(gameData.cameraData.zoom);
            renderer.warm(gameData);
        }));
    reloadConnections.push_back(
        reloader.commands.onReloadScripts.connect([this] { luaScriptSystem.loadScripts(); }));

    luaScriptSystem.bindGameObjects(&playback, &camera, &screenTransition, &world);

    luaScriptSystem.triggerGameLoaded();
}

Game::~Game() = default;

void Game::frame(float deltaTime)
{
    gameUi.commands().drain();

    KeysDown keysDown = keysUnlessCaptured(window.keysDown(), gameUi.wantsKeyboard());
    keys.poll(keysDown);
    if (keys.isPressed(GLFW_KEY_P))
        playback.isPaused() ? playback.play() : playback.pause();
    if (keys.isPressed(GLFW_KEY_F1))
        showEditors = !showEditors;
    if (keys.isPressed(GLFW_KEY_S))
        playback.step();

    luaScriptSystem.update(deltaTime);
    camera.update(deltaTime);
    screenTransition.update(deltaTime);
    gameUi.update(deltaTime, world.getLevel(), world.getLevelData(), world.getLevelPath(), camera);

    playback.advance(
        deltaTime,
        [this, &keysDown]
        {
            keyboardIntentions.process(keysDown);
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
    renderer.draw(
        camera.getProjection(),
        world.getLevel().getTileMap(),
        world.getLevel().getPickups(),
        world.getLevel().getNpcs(),
        world.getPlayer());

    gameUi.draw(
        GameUiSubject{
            gameData,
            world.getLevel(),
            world.getLevelData(),
            world.getLevelPath(),
            world.getPlayer(),
            renderer.getTextures(),
            gameData.levels,
            camera,
            world.getScore(),
            playback.isPaused(),
            showEditors});

    renderer.draw(screenTransition);
}
