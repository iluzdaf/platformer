#include <memory>
#include <cstdlib>
#include <optional>
#include <stdexcept>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <utility>
#include "game/game.hpp"
#include "game/game_data.hpp"
#include "rendering/ui/game_ui.hpp"
#include "cameras/camera2d.hpp"
#include "actor/actor.hpp"
#include "game/level.hpp"
#include "game/levels.hpp"
#include "rendering/screen_transition.hpp"
#include "npc/npc_spawn_data.hpp"
#include "npc/npc.hpp"
#include "assets/asset_paths.hpp"
#include "window/window.hpp"
#include "scripting/lua_script_system.hpp"

Game::Game(Window &window)
    : window(window), gameData(loadGameData()),
      camera(gameData.cameraData, window.getFramebufferSize().x, window.getFramebufferSize().y),
      gameUi(window, window.getFramebufferSize().x, window.getFramebufferSize().y),
      levels(assets::pathTo(assets::LevelList))
{
    window.setSize(gameData.settings.windowWidth, gameData.settings.windowHeight);
    window.onResize.connect([this](int width, int height) { resize(width, height); });

    keyboardManager.registerKey(GLFW_KEY_P);
    keyboardManager.registerKey(GLFW_KEY_S);
    keyboardManager.registerKey(GLFW_KEY_F1);
    showEditors = gameData.settings.debug;
    loadLevel(levels.getFirst());

    gameUi.commands().onPlay.connect([this] { playback.play(); });
    gameUi.commands().onPause.connect([this] { playback.pause(); });
    gameUi.commands().onStep.connect([this] { playback.step(); });
    gameUi.commands().onLoadLevel.connect([this](const std::string &levelPath)
                                          { loadLevel(levelPath); });
    gameUi.commands().onRespawn.connect([this] { rebuildPlayer(); });
    gameUi.commands().onSettingsChanged.connect(
        [this]
        { this->window.setSize(gameData.settings.windowWidth, gameData.settings.windowHeight); });
    gameUi.commands().onCameraChanged.connect([this] { camera.setZoom(gameData.cameraData.zoom); });
    gameUi.commands().onSetFirstLevel.connect(
        [this]
        {
            levels.setFirst(level->getPath());
            levels.save();
        });

    luaScriptSystem.bindGameObjects(this, &playback, &camera, &screenTransition);

    luaScriptSystem.triggerGameLoaded();
}

Game::~Game() = default;

bool Game::isPlaying(const std::string &levelPath) const
{
    return level && level->getPath() == levelPath;
}

void Game::reloadShader(const std::string &shaderPath)
{
    renderer.reloadShader(shaderPath);
}

void Game::reloadTexture(const std::string &texturePath)
{
    renderer.reloadTexture(texturePath);
}

void Game::reloadScripts()
{
    luaScriptSystem.loadScripts();
}

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
    gameUi.update(deltaTime, *level.get(), camera);

    playback.advance(
        deltaTime,
        [this] { preFixedUpdate(); },
        [this](float dt)
        {
            fixedUpdate(dt);
            postFixedUpdate();
        },
        [this](float dt) { update(dt); });

    camera.follow(player->getPosition());

    render();
}

void Game::preFixedUpdate()
{
    inputManager.process(window.getHandle());

    for (Actor *actor : actors)
        actor->preFixedUpdate();
}

void Game::fixedUpdate(float deltaTime)
{
    std::optional<glm::vec2> playerPosition;
    if (player)
        playerPosition = player->getPhysicsBody().getAABB().bottomCenter();

    for (Actor *actor : actors)
        actor->fixedUpdate(
            deltaTime, *level.get(), actor == player.get() ? std::nullopt : playerPosition);

    tileInteractionSystem.fixedUpdate(*player.get(), level->getTileMap());
}

void Game::postFixedUpdate()
{
    for (Actor *actor : actors)
        actor->postFixedUpdate();
}

void Game::update(float deltaTime)
{
    level->getTileMap().update(deltaTime);
}

void Game::render()
{
    renderer.draw(camera.getProjection(), level->getTileMap(), actors);

    gameUi.draw(
        GameUiSubject{
            gameData,
            *level.get(),
            npcs,
            *player.get(),
            renderer.getTileSet(),
            levels.getFirst(),
            camera,
            scoringSystem,
            playback.isPaused(),
            showEditors});

    renderer.draw(screenTransition);
}

void Game::resize(int windowWidth, int windowHeight)
{
    camera.resize(windowWidth, windowHeight);

    gameUi.resize(windowWidth, windowHeight);
}

void Game::reload()
{
    gameData = loadGameData();
    gameUi.forget();

    window.setSize(gameData.settings.windowWidth, gameData.settings.windowHeight);

    camera.setZoom(gameData.cameraData.zoom);

    loadLevel(level ? level->getPath() : levels.getFirst());
}

void Game::loadLevel(const std::string &levelPath)
{
    rebuildLevel(levelPath);

    camera.setWorldBounds(
        glm::vec2(0),
        glm::vec2(level->getTileMap().getWorldWidth(), level->getTileMap().getWorldHeight()));

    rebuildPlayer();

    rebuildNpcs();
}

void Game::rebuildLevel(const std::string &levelPath)
{
    std::unique_ptr<Level> newLevel = std::make_unique<Level>(
        levelPath, gameData.tilePalettes, gameData.playerData, gameData.npcData);
    level = std::move(newLevel);
    luaScriptSystem.bindLevel(level.get());
}

void Game::rebuildPlayer()
{
    if (!level)
        throw std::runtime_error("Cannot rebuild the player before the tile map");

    std::unique_ptr<Player> newPlayer = std::make_unique<Player>(gameData.playerData, inputManager);
    player = std::move(newPlayer);
    player->setPosition(
        level->getTileMap().tileToBottomCenterPosition(level->getPlayerStartTile()) -
        player->getPhysicsBody().getBottomCenterOffset());
    player->onDeath.connect([this] { luaScriptSystem.triggerDeath(); });
    onLevelCompleteConnection = player->onLevelComplete.connect(
        [this]()
        {
            onLevelCompleteConnection.block();
            luaScriptSystem.triggerLevelComplete();
        });
    player->onWallJump.connect([this] { luaScriptSystem.triggerWallJump(); });
    player->onDash.connect([this] { luaScriptSystem.triggerDash(); });
    player->onWallSliding.connect([this] { luaScriptSystem.triggerWallSliding(); });
    player->onFallFromHeight.connect([this] { luaScriptSystem.triggerFallFromHeight(); });
    player->onHitCeiling.connect([this] { luaScriptSystem.triggerHitCeiling(); });
    player->onPickup.connect([this](int scoreDelta) { scoringSystem.addScore(scoreDelta); });
    luaScriptSystem.bindPlayer(player.get());

    refreshActors();
}

void Game::rebuildNpcs()
{
    npcs.clear();

    for (const NpcSpawnData &spawn : level->getNpcs())
    {
        auto it = gameData.npcData.find(spawn.type);

        std::unique_ptr<Npc> newNpc = std::make_unique<Npc>(it->second, level->patrolFor(spawn));
        newNpc->setPosition(
            level->getTileMap().tileToBottomCenterPosition(spawn.tilePosition) -
            newNpc->getPhysicsBody().getBottomCenterOffset());
        npcs.push_back(std::move(newNpc));
    }

    refreshActors();
}

void Game::refreshActors()
{
    actors.clear();

    for (auto &npc : npcs)
        actors.push_back(npc.get());

    if (player)
        actors.push_back(player.get());
}
