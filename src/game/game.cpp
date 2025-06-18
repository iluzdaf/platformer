#include <glaze/glaze.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include "game/game.hpp"
#include "rendering/shader_data.hpp"

Game::Game()
{
    GameData gameData = loadGameData();
    shouldDrawGrid = gameData.debugData.shouldDrawGrid;
    shouldDrawTileInfo = gameData.debugData.shouldDrawTileInfo;
    shouldDrawPlayerAABBs = gameData.debugData.shouldDrawPlayerAABBs;
    shouldDrawTileMapAABBs = gameData.debugData.shouldDrawTileMapAABBs;
    showDebug = gameData.debugData.showDebug;
    showTileMapEditor = gameData.debugData.showTileMapEditor;

    initGLFW(gameData.windowWidth, gameData.windowHeight);

    initGlad();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    luaScriptSystem = std::make_unique<LuaScriptSystem>();
    int windowWidth, windowHeight;
    glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
    camera = std::make_unique<Camera2D>(gameData.cameraData, windowWidth, windowHeight);
    keyboardManager.registerKey(GLFW_KEY_UP);
    keyboardManager.registerKey(GLFW_KEY_LEFT);
    keyboardManager.registerKey(GLFW_KEY_RIGHT);
    keyboardManager.registerKey(GLFW_KEY_RIGHT_SHIFT);
    keyboardManager.registerKey(GLFW_KEY_P);
    keyboardManager.registerKey(GLFW_KEY_S);
    levelWatcher.onLevelChanged.connect([this](const std::string &levelPath)
                                        {
        if (levelPath.compare(tileMap->getLevel()) == 0)
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
            if (shaderPath.compare("../assets/shaders/tile_set.vs") == 0 || shaderPath.compare("../assets/shaders/tile_set.fs") == 0)
            {
                shaderData.vertexPath = "../assets/shaders/tile_set.vs";
                shaderData.fragmentPath = "../assets/shaders/tile_set.fs";
                std::unique_ptr<Shader> newtileSetShader = std::make_unique<Shader>(shaderData);
                tileSetShader = std::move(newtileSetShader);
            }
            else if (shaderPath.compare("../assets/shaders/transition.vs") == 0 || shaderPath.compare("../assets/shaders/transition.fs") == 0)
            {
                shaderData.vertexPath = "../assets/shaders/transition.vs";
                shaderData.fragmentPath = "../assets/shaders/transition.fs";
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
            if (texturePath.compare("../assets/textures/tile_set.png") == 0)
            {
                std::unique_ptr<Texture2D> newTileSet = std::make_unique<Texture2D>("../assets/textures/tile_set.png");
                tileSet = std::move(newTileSet);                                                        
            }
            else if (texturePath.compare("../assets/textures/player.png") == 0)
            {
                std::unique_ptr<Texture2D> newPlayerTexture = std::make_unique<Texture2D>("../assets/textures/player.png");
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
                                           { luaScriptSystem->loadScripts(); });

    player = std::make_unique<Player>(gameData.playerData, gameData.physicsData);
    player->onDeath.connect([this]
                            { luaScriptSystem->triggerDeath(); });
    onLevelCompleteConnection = player->onLevelComplete.connect([this]()
                                                                {
        onLevelCompleteConnection.block();
        luaScriptSystem->triggerLevelComplete(); });
    player->onWallJump.connect([this]
                               { luaScriptSystem->triggerWallJump(); });
    player->onDoubleJump.connect([this]
                                 { luaScriptSystem->triggerDoubleJump(); });
    player->onDash.connect([this]
                           { luaScriptSystem->triggerDash(); });
    player->onFallFromHeight.connect([this]
                                     { luaScriptSystem->triggerFallFromHeight(); });
    player->onHitCeiling.connect([this]
                                 { luaScriptSystem->triggerHitCeiling(); });
    player->onWallSliding.connect([this]
                                  { luaScriptSystem->triggerWallSliding(); });
    player->onPickup.connect([this](int scoreDelta)
                             { scoringSystem.addScore(scoreDelta); });
    loadLevel(gameData.firstLevel);

    tileSet = std::make_unique<Texture2D>("../assets/textures/tile_set.png");
    ShaderData shaderData;
    shaderData.vertexPath = "../assets/shaders/tile_set.vs";
    shaderData.fragmentPath = "../assets/shaders/tile_set.fs";
    tileSetShader = std::make_unique<Shader>(shaderData);
    spriteRenderer = std::make_unique<SpriteRenderer>();
    tileMapRenderer = std::make_unique<TileMapRenderer>(*spriteRenderer.get());
    playerTexture = std::make_unique<Texture2D>("../assets/textures/player.png");
    shaderData.vertexPath = "../assets/shaders/transition.vs";
    shaderData.fragmentPath = "../assets/shaders/transition.fs";
    screenTransitionShader = std::make_unique<Shader>(shaderData);
    screenTransition = std::make_unique<ScreenTransition>();
    debugUi.onPlay.connect([this]
                           { play(); });
    debugUi.onStep.connect([this]
                           { step(); });
    debugUi.onRespawn.connect([this]
                              {
        player->reset();
        player->setPosition(tileMap->getPlayerStartWorldPosition()); });
    debugUi.onToggleZoom.connect([this]
                                 {
        static int originalZoom = camera->getZoom();
        int currentZoom = camera->getZoom();
        camera->setZoom(currentZoom == originalZoom ? 3 : originalZoom); });
    debugUi.onToggleDrawGrid.connect([this]
                                     { shouldDrawGrid = !shouldDrawGrid; });
    debugUi.onToggleDrawTileInfo.connect([this]
                                         { shouldDrawTileInfo = !shouldDrawTileInfo; });
    debugUi.onToggleDrawPlayerAABBs.connect([this]
                                            { shouldDrawPlayerAABBs = !shouldDrawPlayerAABBs; });
    debugUi.onToggleDrawTileMapAABBs.connect([this]
                                             { shouldDrawTileMapAABBs = !shouldDrawTileMapAABBs; });
    debugUi.onGameReload.connect([this]
                                 { reload(); });
    imGuiManager = std::make_unique<ImGuiManager>(window, windowWidth, windowHeight);
    editorTileMapUi.onLoadLevel.connect([this](const std::string &levelPath)
                                        { loadLevel(levelPath); });

    luaScriptSystem->bindGameObjects(this, camera.get(), player.get(), screenTransition.get());

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

void Game::fixedUpdate(float deltaTime)
{
    player->fixedUpdate(deltaTime, *tileMap.get());

    tileInteractionSystem.fixedUpdate(*player.get(), *tileMap.get());
}

void Game::update(float deltaTime)
{
    player->update(deltaTime, *tileMap.get());

    tileMap->update(deltaTime);
}

void Game::render()
{
    glClearColor(0.1f, 0.12f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glm::mat4 projection = camera->getProjection();

    tileMapRenderer->draw(
        *tileMap.get(),
        projection,
        *tileSetShader.get(),
        *tileSet.get());

    PlayerState playerState = player->getPlayerState();
    spriteRenderer->drawWithUV(
        *tileSetShader.get(),
        *playerTexture.get(),
        projection,
        player->getPosition(),
        playerState.size,
        playerState.currentAnimationUVStart,
        playerState.currentAnimationUVEnd,
        playerState.facingLeft);

    imGuiManager->newFrame();

    scoreUi.draw(
        *imGuiManager.get(),
        scoringSystem,
        *tileSet.get());

    imGuiManager->render();

    screenTransition->draw(*screenTransitionShader.get());

    imGuiManager->newFrame();

    debugTileMapUi.draw(
        *imGuiManager.get(),
        *camera.get(),
        *tileMap.get(),
        shouldDrawGrid,
        shouldDrawTileInfo);

    debugAABBUi.draw(
        *imGuiManager.get(),
        *player.get(),
        *tileMap.get(),
        *camera.get(),
        shouldDrawPlayerAABBs,
        shouldDrawTileMapAABBs);

    debugUi.draw(
        *imGuiManager.get(),
        playerState,
        *camera.get(),
        showDebug);

    editorTileMapUi.draw(
        *imGuiManager.get(),
        *tileMap.get(),
        *tileSet.get(),
        showTileMapEditor);

    imGuiManager->render();
}

void Game::resize(int windowWidth, int windowHeight)
{
    camera->resize(windowWidth, windowHeight);
    imGuiManager->resize(windowWidth, windowHeight);
}

void Game::run()
{
    float lastTime = glfwGetTime();
    while (!glfwWindowShouldClose(window))
    {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        levelWatcher.process();
        assetWatcher.process();
        gameDataWatcher.process();
        scriptWatcher.process();
        keyboardManager.update(window);
        luaScriptSystem->update(deltaTime);
        camera->update(deltaTime);
        screenTransition->update(deltaTime);
        debugAABBUi.update(deltaTime);
        editorTileMapUi.update(
            *imGuiManager.get(),
            *camera.get(),
            *tileMap.get());

        if (keyboardManager.isPressed(GLFW_KEY_P))
            play();
        if (keyboardManager.isPressed(GLFW_KEY_S))
            step();

        if (!paused || stepFrame)
        {
            preFixedUpdate();

            if (stepFrame)
            {
                fixedUpdate(0.01f);
                update(0.01f);
            }
            else
            {
                timestepper.run(deltaTime, [&](float dt)
                                { fixedUpdate(dt); });
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

void Game::initGLFW(int windowWidth, int windowHeight)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(windowWidth, windowHeight, "platformer", NULL, NULL);
    if (!window)
    {
        throw std::runtime_error("Failed to create window");
    }

    glfwMakeContextCurrent(window);

    glfwSwapInterval(1);

    glfwSetWindowUserPointer(window, this);

    glfwSetFramebufferSizeCallback(window, [](GLFWwindow *window, int windowWidth, int windowHeight)
                                   {
        if (Game *game = static_cast<Game *>(glfwGetWindowUserPointer(window)))
        {
            game->resize(windowWidth, windowHeight);
        } });
}

GameData Game::loadGameData() const
{
    GameData gameData;
    auto ec = glz::read_file_json(gameData, "../assets/game_data.json", std::string{});
    if (ec)
    {
        throw std::runtime_error("Failed to read game data json file");
    }

    return gameData;
}

void Game::initGlad()
{
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        throw std::runtime_error("Failed to initialize GLAD");
    }
}

void Game::preFixedUpdate()
{
    player->preFixedUpdate();

    static bool wasJumpKeyDownLastFrame = false;
    bool isJumpKeyDown = keyboardManager.isDown(GLFW_KEY_UP);
    if (isJumpKeyDown && !wasJumpKeyDownLastFrame)
        player->jump();
    wasJumpKeyDownLastFrame = isJumpKeyDown;

    if (keyboardManager.isDown(GLFW_KEY_LEFT))
        player->moveLeft();
    if (keyboardManager.isDown(GLFW_KEY_RIGHT))
        player->moveRight();

    static bool wasDashKeyDownLastFrame = false;
    bool isDashKeyDown = keyboardManager.isDown(GLFW_KEY_RIGHT_SHIFT);
    if (isDashKeyDown && !wasDashKeyDownLastFrame)
        player->dash();
    wasDashKeyDownLastFrame = isDashKeyDown;
}

void Game::loadLevel(const std::string &levelPath)
{
    std::unique_ptr<TileMap> newTileMap = std::make_unique<TileMap>(levelPath);
    tileMap = std::move(newTileMap);
    luaScriptSystem->bindTileMap(tileMap.get());
    camera->setWorldBounds(glm::vec2(0), glm::vec2(tileMap->getWorldWidth(), tileMap->getWorldHeight()));
    onLevelCompleteConnection.unblock();
    player->reset();
    player->setPosition(tileMap->getPlayerStartWorldPosition());
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

void Game::reload()
{
    GameData gameData = loadGameData();

    shouldDrawGrid = gameData.debugData.shouldDrawGrid;
    shouldDrawTileInfo = gameData.debugData.shouldDrawTileInfo;
    shouldDrawPlayerAABBs = gameData.debugData.shouldDrawPlayerAABBs;
    shouldDrawTileMapAABBs = gameData.debugData.shouldDrawTileMapAABBs;
    showDebug = gameData.debugData.showDebug;
    showTileMapEditor = gameData.debugData.showTileMapEditor;

    glfwSetWindowSize(window, gameData.windowWidth, gameData.windowHeight);

    camera->setZoom(gameData.cameraData.zoom);

    player->initFromData(gameData.playerData, gameData.physicsData);

    loadLevel(gameData.firstLevel);
}