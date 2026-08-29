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
#include "actor/actor_state.hpp"
#include "game/level.hpp"
#include "game/levels.hpp"
#include "rendering/screen_transition.hpp"
#include "rendering/shader.hpp"
#include "npc/npc_spawn_data.hpp"
#include "npc/npc.hpp"
#include "rendering/shader_data.hpp"
#include "assets/asset_paths.hpp"
#include "window/window.hpp"
#include "scripting/lua_script_system.hpp"
#include "rendering/sprite_renderer.hpp"
#include "rendering/tile_map_drawing.hpp"
#include "rendering/texture2d.hpp"

Game::Game(Window &window)
    : window(window), gameData(loadGameData()),
      camera(gameData.cameraData, window.getFramebufferSize().x, window.getFramebufferSize().y),
      gameUi(window, window.getFramebufferSize().x, window.getFramebufferSize().y),
      levels(assets::pathTo(assets::LevelList))
{
    window.setSize(gameData.settings.windowWidth, gameData.settings.windowHeight);
    window.onResize.connect([this](int width, int height) { resize(width, height); });

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    keyboardManager.registerKey(GLFW_KEY_P);
    keyboardManager.registerKey(GLFW_KEY_S);
    keyboardManager.registerKey(GLFW_KEY_F1);
    showEditors = gameData.settings.debug;
    loadLevel(levels.getFirst());

    reloadTexture(std::string(assets::TileSetTexture));
    reloadTexture(std::string(assets::PlayerTexture));
    reloadShader(std::string(assets::TileSetVertexShader));
    reloadShader(std::string(assets::TransitionVertexShader));
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
    if (shaderPath == assets::TileSetVertexShader || shaderPath == assets::TileSetFragmentShader)
        tileSetShader = loadShader(assets::TileSetVertexShader, assets::TileSetFragmentShader);
    else if (
        shaderPath == assets::TransitionVertexShader ||
        shaderPath == assets::TransitionFragmentShader)
        screenTransitionShader =
            loadShader(assets::TransitionVertexShader, assets::TransitionFragmentShader);
}

void Game::reloadTexture(const std::string &texturePath)
{
    if (texturePath == assets::TileSetTexture)
        tileSet = std::make_unique<Texture2D>(assets::pathTo(assets::TileSetTexture));
    else if (texturePath == assets::PlayerTexture)
        playerTexture = std::make_unique<Texture2D>(assets::pathTo(assets::PlayerTexture));
}

void Game::reloadScripts()
{
    luaScriptSystem.loadScripts();
}

std::unique_ptr<Shader> Game::loadShader(std::string_view vertex, std::string_view fragment) const
{
    ShaderData shaderData;
    shaderData.vertexPath = assets::pathTo(vertex);
    shaderData.fragmentPath = assets::pathTo(fragment);

    return std::make_unique<Shader>(shaderData);
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
    glClearColor(0.1f, 0.12f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glm::mat4 projection = camera.getProjection();

    drawTileMap(
        spriteRenderer, level->getTileMap(), projection, *tileSetShader.get(), *tileSet.get());

    for (const Actor *actor : actors)
    {
        const ActorState &actorState = actor->getState();
        spriteRenderer.drawWithUV(
            *tileSetShader.get(),
            *playerTexture.get(),
            projection,
            actor->getPosition(),
            actorState.size,
            actorState.currentAnimationUVStart,
            actorState.currentAnimationUVEnd,
            actorState.facingLeft);
    }

    gameUi.draw(
        GameUiSubject{
            gameData,
            *level.get(),
            npcs,
            *player.get(),
            *tileSet.get(),
            levels.getFirst(),
            camera,
            scoringSystem,
            playback.isPaused(),
            showEditors});

    screenTransition.draw(*screenTransitionShader.get());
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
