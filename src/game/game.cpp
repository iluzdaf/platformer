#include <glaze/glaze.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include "game/game.hpp"

Game::Game()
{
    GameData gameData = loadGameData();
    shouldDrawGrid = gameData.debugData.shouldDrawGrid;
    shouldDrawTileInfo = gameData.debugData.shouldDrawTileInfo;
    shouldDrawPlayerAABBs = gameData.debugData.shouldDrawPlayerAABBs;
    shouldDrawTileMapAABBs = gameData.debugData.shouldDrawTileMapAABBs;
    showDebugControls = gameData.debugData.showDebugControls;
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

    player = std::make_unique<Player>(gameData.playerData, gameData.physicsData);
    player->onDeath.connect([this]
                            { luaScriptSystem->triggerDeath(); });
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
    loadLevel(gameData.firstLevel);
    tileInteractionSystem = std::make_unique<TileInteractionSystem>();

    tileSet = std::make_unique<Texture2D>("../assets/textures/tile_set.png");
    tileSetShader.initByShaderFile("../assets/shaders/tile_set.vs", "../assets/shaders/tile_set.fs");
    tileSetSpriteRenderer = std::make_unique<SpriteRenderer>(tileSetShader);
    tileMapRenderer = std::make_unique<TileMapRenderer>(*tileSet.get(), *tileSetSpriteRenderer.get());
    playerTexture = std::make_unique<Texture2D>("../assets/textures/player.png");
    screenTransitionShader.initByShaderFile("../assets/shaders/transition.vs", "../assets/shaders/transition.fs");
    screenTransition = std::make_unique<ScreenTransition>(screenTransitionShader);
    debugControlUi = std::make_unique<DebugControlUi>();
    debugControlUi->onPlay.connect([this]
                                   { play(); });
    debugControlUi->onStep.connect([this]
                                   { step(); });
    debugControlUi->onRespawn.connect([this]
                                      { player->setPosition(tileMap->getPlayerStartWorldPosition()); });
    debugControlUi->onToggleZoom.connect([this]
                                         {
        static int originalZoom = camera->getZoom();
        int currentZoom = camera->getZoom();
        camera->setZoom(currentZoom == originalZoom? 3 : originalZoom); });
    debugControlUi->onToggleDrawGrid.connect([this]
                                             { shouldDrawGrid = !shouldDrawGrid; });
    debugControlUi->onToggleDrawTileInfo.connect([this]
                                                 { shouldDrawTileInfo = !shouldDrawTileInfo; });
    debugControlUi->onToggleDrawPlayerAABBs.connect([this]
                                                    { shouldDrawPlayerAABBs = !shouldDrawPlayerAABBs; });
    debugControlUi->onToggleDrawTileMapAABBs.connect([this]
                                                     { shouldDrawTileMapAABBs = !shouldDrawTileMapAABBs; });
    debugControlUi->onGameReload.connect([this]
                                         { reload(); });
    imGuiManager = std::make_unique<ImGuiManager>(window, windowWidth, windowHeight);
    debugTileMapUi = std::make_unique<DebugTileMapUi>();
    debugAABBUi = std::make_unique<DebugAABBUi>();
    editorTileMapUi = std::make_unique<EditorTileMapUi>();
    editorTileMapUi->onLoadLevel.connect([this](const std::string &levelPath)
                                         { loadLevel(levelPath); });

    luaScriptSystem->bindGameObjects(this, camera.get(), player.get(), screenTransition.get());

    screenTransition->start(0.4f, true);
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

    tileInteractionSystem->fixedUpdate(*player.get(), *tileMap.get());
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

    tileMapRenderer->draw(*tileMap.get(), projection);

    PlayerState playerState = player->getPlayerState();
    tileSetSpriteRenderer->drawWithUV(
        *playerTexture.get(),
        projection,
        playerState.position,
        playerState.size,
        playerState.currentAnimationUVStart,
        playerState.currentAnimationUVEnd,
        playerState.facingLeft);

    screenTransition->draw();

    imGuiManager->newFrame();

    debugTileMapUi->draw(
        *imGuiManager.get(),
        *camera.get(),
        *tileMap.get(),
        shouldDrawGrid,
        shouldDrawTileInfo);

    debugAABBUi->draw(
        *imGuiManager.get(),
        *player.get(),
        *tileMap.get(),
        *camera.get(),
        shouldDrawPlayerAABBs,
        shouldDrawTileMapAABBs);

    debugControlUi->draw(showDebugControls);

    editorTileMapUi->draw(
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

        keyboardManager.update(window);
        luaScriptSystem->update(deltaTime);
        camera->update(deltaTime);
        screenTransition->update(deltaTime);
        imGuiManager->update();
        debugAABBUi->update(deltaTime);
        editorTileMapUi->update(
            *imGuiManager.get(),
            *camera.get(),
            *tileMap.get());

        if (keyboardManager.isPressed(GLFW_KEY_P))
        {
            play();
        }
        if (keyboardManager.isPressed(GLFW_KEY_S))
        {
            step();
        }

        if (!paused || stepFrame)
        {
            preFixedUpdate();
            timestepper.run(deltaTime, [&](float dt)
                            { fixedUpdate(dt); });
            update(deltaTime);
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

    if (keyboardManager.isPressed(GLFW_KEY_UP))
    {
        player->jump();
    }
    if (keyboardManager.isDown(GLFW_KEY_LEFT))
    {
        player->moveLeft();
    }
    if (keyboardManager.isDown(GLFW_KEY_RIGHT))
    {
        player->moveRight();
    }
    if (keyboardManager.isPressed(GLFW_KEY_RIGHT_SHIFT))
    {
        player->dash();
    }
}

void Game::loadLevel(const std::string &levelPath)
{
    tileMap = std::make_unique<TileMap>(levelPath);
    luaScriptSystem->bindTileMap(tileMap.get());
    camera->setWorldBounds(glm::vec2(0), glm::vec2(tileMap->getWorldWidth(), tileMap->getWorldHeight()));
    onLevelCompleteConnection.disconnect();
    onLevelCompleteConnection = player->onLevelComplete.connect([this]()
                                                                {
        onLevelCompleteConnection.disconnect();
        luaScriptSystem->triggerLevelComplete(); });
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
    showDebugControls = gameData.debugData.showDebugControls;
    showTileMapEditor = gameData.debugData.showTileMapEditor;

    glfwSetWindowSize(window, gameData.windowHeight, gameData.windowHeight);

    luaScriptSystem->loadScripts();

    loadLevel(gameData.firstLevel);

    player->initFromData(gameData.playerData, gameData.physicsData);

    camera->setZoom(gameData.cameraData.zoom);
}