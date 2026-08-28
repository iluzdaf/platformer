#include <stdexcept>
#include <glaze/glaze.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include "game/game.hpp"
#include "game/level.hpp"
#include "game/levels.hpp"
#include "rendering/shader_data.hpp"
#include "assets/asset_paths.hpp"

Game::Game(Window &window) : window(window), levels(assets::pathTo(assets::LevelList))
{
    gameData = loadGameData();

    window.setSize(gameData.windowWidth, gameData.windowHeight);
    window.onResize.connect([this](int width, int height) { resize(width, height); });

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    luaScriptSystem = std::make_unique<LuaScriptSystem>();
    glm::ivec2 framebuffer = window.getFramebufferSize();
    camera = std::make_unique<Camera2D>(gameData.cameraData, framebuffer.x, framebuffer.y);
    keyboardManager.registerKey(GLFW_KEY_P);
    keyboardManager.registerKey(GLFW_KEY_S);
    loadLevel(levels.getFirst());

    reloadTexture(std::string(assets::TileSetTexture));
    reloadTexture(std::string(assets::PlayerTexture));
    reloadShader(std::string(assets::TileSetVertexShader));
    reloadShader(std::string(assets::TransitionVertexShader));
    spriteRenderer = std::make_unique<SpriteRenderer>();
    tileMapRenderer = std::make_unique<TileMapRenderer>(*spriteRenderer.get());
    screenTransition = std::make_unique<ScreenTransition>();
    gameEditorUi.onPlay.connect([this] { play(); });
    gameEditorUi.onStep.connect([this] { step(); });
    gameEditorUi.onToggleZoom.connect(
        [this]
        {
            static float originalZoom = camera->getZoom();
            float currentZoom = camera->getZoom();
            camera->setZoom(std::abs(currentZoom - originalZoom) < 1e-5f ? 3.0f : originalZoom);
        });
    imGuiManager = std::make_unique<ImGuiManager>(window.getHandle(), framebuffer.x, framebuffer.y);
    levelEditorUi.onLoadLevel.connect([this](const std::string &levelPath)
                                      { loadLevel(levelPath); });
    levelEditorUi.onRespawn.connect([this] { rebuildPlayer(); });
    levelEditorUi.onSetFirstLevel.connect(
        [this]
        {
            levels.setFirst(level->getPath());
            levels.save();
        });

    luaScriptSystem->bindGameObjects(this, camera.get(), screenTransition.get());

    luaScriptSystem->triggerGameLoaded();
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
    luaScriptSystem->loadScripts();
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
        play();
    if (keyboardManager.isPressed(GLFW_KEY_S))
        step();

    luaScriptSystem->update(deltaTime);
    camera->update(deltaTime);
    screenTransition->update(deltaTime);
    debugAABBUi.update(deltaTime);
    levelEditorUi.update(*imGuiManager.get(), *camera.get(), *level.get());

    if (!paused || stepFrame)
    {
        preFixedUpdate();

        if (stepFrame)
        {
            float dt = std::min(deltaTime, 0.01f);
            fixedUpdate(dt);
            postFixedUpdate();
            update(dt);
        }
        else
        {
            timestepper.run(
                deltaTime,
                [&](float dt)
                {
                    fixedUpdate(dt);
                    postFixedUpdate();
                });
            update(deltaTime);
        }

        stepFrame = false;
    }

    camera->follow(player->getPosition());

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

    glm::mat4 projection = camera->getProjection();

    tileMapRenderer->draw(level->getTileMap(), projection, *tileSetShader.get(), *tileSet.get());

    for (const Actor *actor : actors)
    {
        const ActorState &actorState = actor->getState();
        spriteRenderer->drawWithUV(
            *tileSetShader.get(),
            *playerTexture.get(),
            projection,
            actor->getPosition(),
            actorState.size,
            actorState.currentAnimationUVStart,
            actorState.currentAnimationUVEnd,
            actorState.facingLeft);
    }

    imGuiManager->newFrame();

    scoreUi.draw(*imGuiManager.get(), scoringSystem, *tileSet.get());

    levelEditorUi.draw(
        *imGuiManager.get(),
        *level.get(),
        *tileSet.get(),
        *camera.get(),
        levels.getFirst(),
        gameData.debug);

    gameEditorUi.draw(
        *imGuiManager.get(),
        player->getMotion().getState(),
        player->getPosition(),
        player->getState(),
        *camera.get(),
        gameData.debug);

    debugAABBUi.draw(
        *imGuiManager.get(),
        *player.get(),
        level->getTileMap(),
        level->getPlayerStartTile(),
        *camera.get(),
        gameEditorUi.drawsPlayerAABBs(),
        levelEditorUi.drawsTileMapAABBs());

    imGuiManager->render();

    screenTransition->draw(*screenTransitionShader.get());
}

void Game::resize(int windowWidth, int windowHeight)
{
    camera->resize(windowWidth, windowHeight);

    imGuiManager->resize(windowWidth, windowHeight);
}

GameData Game::loadGameData() const
{
    GameData loaded;
    auto ec = glz::read_file_json(loaded, assets::pathTo(assets::GameData), std::string{});
    if (ec)
        throw std::runtime_error("Failed to read game data json file");
    return loaded;
}

void Game::reload()
{
    gameData = loadGameData();

    window.setSize(gameData.windowWidth, gameData.windowHeight);

    camera->setZoom(gameData.cameraData.zoom);

    loadLevel(level ? level->getPath() : levels.getFirst());
}

void Game::loadLevel(const std::string &levelPath)
{
    rebuildLevel(levelPath);

    camera->setWorldBounds(
        glm::vec2(0),
        glm::vec2(level->getTileMap().getWorldWidth(), level->getTileMap().getWorldHeight()));

    rebuildPlayer();

    rebuildNpcs();
}

void Game::pause()
{
    paused = true;
}

void Game::step()
{
    paused = true;
    stepFrame = true;
}

void Game::play()
{
    paused = false;
    stepFrame = false;
}

void Game::rebuildLevel(const std::string &levelPath)
{
    std::unique_ptr<Level> newLevel = std::make_unique<Level>(
        levelPath, gameData.tilePalettes, gameData.playerData, gameData.npcData);
    level = std::move(newLevel);
    luaScriptSystem->bindLevel(level.get());
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
    player->onDeath.connect([this] { luaScriptSystem->triggerDeath(); });
    onLevelCompleteConnection = player->onLevelComplete.connect(
        [this]()
        {
            onLevelCompleteConnection.block();
            luaScriptSystem->triggerLevelComplete();
        });
    player->onWallJump.connect([this] { luaScriptSystem->triggerWallJump(); });
    player->onDash.connect([this] { luaScriptSystem->triggerDash(); });
    player->onWallSliding.connect([this] { luaScriptSystem->triggerWallSliding(); });
    player->onFallFromHeight.connect([this] { luaScriptSystem->triggerFallFromHeight(); });
    player->onHitCeiling.connect([this] { luaScriptSystem->triggerHitCeiling(); });
    player->onPickup.connect([this](int scoreDelta) { scoringSystem.addScore(scoreDelta); });
    luaScriptSystem->bindPlayer(player.get());

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
