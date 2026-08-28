#include <stdexcept>
#include <glaze/glaze.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include "game/game.hpp"
#include "game/level.hpp"
#include "game/levels.hpp"
#include "rendering/shader_data.hpp"

Game::Game()
    : levels("../../assets/levels.json")
{
    gameData = loadGameData();

    initGLFW(gameData.windowWidth, gameData.windowHeight);

    initGlad();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    luaScriptSystem = std::make_unique<LuaScriptSystem>();
    int windowWidth, windowHeight;
    glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
    camera = std::make_unique<Camera2D>(gameData.cameraData, windowWidth, windowHeight);
    keyboardManager.registerKey(GLFW_KEY_P);
    keyboardManager.registerKey(GLFW_KEY_S);
    levelWatcher.onLevelChanged.connect([this](const std::string &levelPath)
                                        {
        if (levelPath.compare(level->getPath()) == 0)
            try
            {
                loadLevel(levelPath);
            }
            catch (const std::exception &e)
            {
                std::cerr << e.what() << std::endl;
            } });
    assetWatcher.onShaderChanged.connect([this](const std::string &shaderPath)
                                         {
        try
        {
            ShaderData shaderData;
            if (shaderPath.compare("../../assets/shaders/tile_set.vs") == 0 || shaderPath.compare("../../assets/shaders/tile_set.fs") == 0)
            {
                shaderData.vertexPath = "../../assets/shaders/tile_set.vs";
                shaderData.fragmentPath = "../../assets/shaders/tile_set.fs";
                std::unique_ptr<Shader> newtileSetShader = std::make_unique<Shader>(shaderData);
                tileSetShader = std::move(newtileSetShader);
            }
            else if (shaderPath.compare("../../assets/shaders/transition.vs") == 0 || shaderPath.compare("../../assets/shaders/transition.fs") == 0)
            {
                shaderData.vertexPath = "../../assets/shaders/transition.vs";
                shaderData.fragmentPath = "../../assets/shaders/transition.fs";
                std::unique_ptr<Shader> newScreenTransitionShader = std::make_unique<Shader>(shaderData);
                screenTransitionShader = std::move(newScreenTransitionShader);
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << std::endl;
        } });
    assetWatcher.onTextureChanged.connect([this](const std::string &texturePath)
                                          {
        try
        {
            if (texturePath.compare("../../assets/textures/tile_set.png") == 0)
            {
                std::unique_ptr<Texture2D> newTileSet = std::make_unique<Texture2D>("../../assets/textures/tile_set.png");
                tileSet = std::move(newTileSet);
            }
            else if (texturePath.compare("../../assets/textures/player.png") == 0)
            {
                std::unique_ptr<Texture2D> newPlayerTexture = std::make_unique<Texture2D>("../../assets/textures/player.png");
                playerTexture = std::move(newPlayerTexture);
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << std::endl;
        } });
    gameDataWatcher.onGameDataChanged.connect([this]
                                              {
        try
        {
            reload();
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << std::endl;
        } });
    scriptWatcher.onScriptsChanged.connect([this]
                                           {
        try
        {
            luaScriptSystem->loadScripts();
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << std::endl;
        } });
    loadLevel(levels.getFirst());

    tileSet = std::make_unique<Texture2D>("../../assets/textures/tile_set.png");
    ShaderData shaderData;
    shaderData.vertexPath = "../../assets/shaders/tile_set.vs";
    shaderData.fragmentPath = "../../assets/shaders/tile_set.fs";
    tileSetShader = std::make_unique<Shader>(shaderData);
    spriteRenderer = std::make_unique<SpriteRenderer>();
    tileMapRenderer = std::make_unique<TileMapRenderer>(*spriteRenderer.get());
    playerTexture = std::make_unique<Texture2D>("../../assets/textures/player.png");
    shaderData.vertexPath = "../../assets/shaders/transition.vs";
    shaderData.fragmentPath = "../../assets/shaders/transition.fs";
    screenTransitionShader = std::make_unique<Shader>(shaderData);
    screenTransition = std::make_unique<ScreenTransition>();
    gameEditorUi.onPlay.connect([this]
                                { play(); });
    gameEditorUi.onStep.connect([this]
                                { step(); });
    gameEditorUi.onToggleZoom.connect([this]
                                      {
        static float originalZoom = camera->getZoom();
        float currentZoom = camera->getZoom();
        camera->setZoom(std::abs(currentZoom - originalZoom) < 1e-5f ? 3.0f : originalZoom); });
    imGuiManager = std::make_unique<ImGuiManager>(window, windowWidth, windowHeight);
    levelEditorUi.onLoadLevel.connect([this](const std::string &levelPath)
                                      { loadLevel(levelPath); });
    levelEditorUi.onRespawn.connect([this]
                                    { rebuildPlayer(); });
    levelEditorUi.onSetFirstLevel.connect([this]
                                          {
        levels.setFirst(level->getPath());
        levels.save(); });

    luaScriptSystem->bindGameObjects(this, camera.get(), screenTransition.get());

    luaScriptSystem->triggerGameLoaded();
}

Game::~Game()
{
    if (window)
    {
        glfwDestroyWindow(window);
        window = nullptr;
    }

    glfwTerminate();
}

void Game::run()
{
    double lastTime = glfwGetTime();
    while (!glfwWindowShouldClose(window))
    {
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;

        levelWatcher.process();
        assetWatcher.process();
        gameDataWatcher.process();
        scriptWatcher.process();

        keyboardManager.poll(window);
        if (keyboardManager.isPressed(GLFW_KEY_P))
            play();
        if (keyboardManager.isPressed(GLFW_KEY_S))
            step();

        luaScriptSystem->update(deltaTime);
        camera->update(deltaTime);
        screenTransition->update(deltaTime);
        debugAABBUi.update(deltaTime);
        levelEditorUi.update(
            *imGuiManager.get(),
            *camera.get(),
            *level.get());

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
                timestepper.run(deltaTime, [&](float dt)
                                { fixedUpdate(dt); postFixedUpdate(); });
                update(deltaTime);
            }

            stepFrame = false;
        }

        camera->follow(player->getPosition());

        render();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void Game::preFixedUpdate()
{
    inputManager.process(window);

    for (Actor *actor : actors)
        actor->preFixedUpdate();
}

void Game::fixedUpdate(float deltaTime)
{
    for (Actor *actor : actors)
        actor->fixedUpdate(deltaTime, *level.get());

    tileInteractionSystem.fixedUpdate(
        *player.get(),
        level->getTileMap());
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

    tileMapRenderer->draw(
        level->getTileMap(),
        projection,
        *tileSetShader.get(),
        *tileSet.get());

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

    scoreUi.draw(
        *imGuiManager.get(),
        scoringSystem,
        *tileSet.get());

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

void Game::initGLFW(int windowWidth, int windowHeight)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(windowWidth, windowHeight, "platformer", NULL, NULL);
    if (!window)
        throw std::runtime_error("Failed to create window");

    glfwMakeContextCurrent(window);

    glfwSwapInterval(1);

    glfwSetWindowUserPointer(window, this);

    glfwSetFramebufferSizeCallback(window, [](GLFWwindow *window, int windowWidth, int windowHeight)
                                   {
        if (Game *game = static_cast<Game *>(glfwGetWindowUserPointer(window)))
            game->resize(windowWidth, windowHeight); });
}

void Game::initGlad()
{
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        throw std::runtime_error("Failed to initialize GLAD");
}

GameData Game::loadGameData() const
{
    GameData loaded;
    auto ec = glz::read_file_json(loaded, "../../assets/game_data.json", std::string{});
    if (ec)
        throw std::runtime_error("Failed to read game data json file");
    return loaded;
}

void Game::reload()
{
    gameData = loadGameData();

    glfwSetWindowSize(window, gameData.windowWidth, gameData.windowHeight);

    camera->setZoom(gameData.cameraData.zoom);

    loadLevel(level ? level->getPath() : levels.getFirst());
}

void Game::loadLevel(const std::string &levelPath)
{
    rebuildLevel(levelPath);

    camera->setWorldBounds(glm::vec2(0), glm::vec2(level->getTileMap().getWorldWidth(), level->getTileMap().getWorldHeight()));

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
        levelPath,
        gameData.tilePalettes,
        gameData.playerData,
        gameData.npcData);
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
    player->onDeath.connect([this]
                            { luaScriptSystem->triggerDeath(); });
    onLevelCompleteConnection = player->onLevelComplete.connect([this]()
                                                                {
        onLevelCompleteConnection.block();
        luaScriptSystem->triggerLevelComplete(); });
    player->onWallJump.connect([this]
                               { luaScriptSystem->triggerWallJump(); });
    player->onDash.connect([this]
                           { luaScriptSystem->triggerDash(); });
    player->onWallSliding.connect([this]
                                  { luaScriptSystem->triggerWallSliding(); });
    player->onFallFromHeight.connect([this]
                                     { luaScriptSystem->triggerFallFromHeight(); });
    player->onHitCeiling.connect([this]
                                 { luaScriptSystem->triggerHitCeiling(); });
    player->onPickup.connect([this](int scoreDelta)
                             { scoringSystem.addScore(scoreDelta); });
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
