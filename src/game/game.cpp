#include <glaze/glaze.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include "game/game.hpp"

Game::Game()
{
    initGameData();
    initGLFW();
    initGlad();

    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow *window, int screenWidth, int screenHeight)
                                   {
        if (Game *game = static_cast<Game *>(glfwGetWindowUserPointer(window)))
        {
            game->resize(screenWidth, screenHeight);
        } });

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    luaScriptSystem = std::make_unique<LuaScriptSystem>();
    int screenWidth, screenHeight;
    glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
    camera = std::make_unique<Camera2D>(gameData.cameraData, screenWidth, screenHeight);
    keyboardManager.registerKey(GLFW_KEY_UP);
    keyboardManager.registerKey(GLFW_KEY_LEFT);
    keyboardManager.registerKey(GLFW_KEY_RIGHT);
    keyboardManager.registerKey(GLFW_KEY_RIGHT_SHIFT);
    keyboardManager.registerKey(GLFW_KEY_P);
    keyboardManager.registerKey(GLFW_KEY_S);

    tileMap = std::make_unique<TileMap>((gameData.firstLevel));
    camera->setWorldBounds(glm::vec2(0), glm::vec2(tileMap->getWorldWidth(), tileMap->getWorldHeight()));
    player = std::make_unique<Player>(gameData.playerData, gameData.physicsData);
    player->setPosition(tileMap->getPlayerStartWorldPosition());
    onLevelCompleteConnection = player->onLevelComplete.connect([this]()
                                                                {
        onLevelCompleteConnection.disconnect();
        luaScriptSystem->triggerLevelComplete(); });
    player->onDeath.connect([this]()
                            { luaScriptSystem->triggerRespawn(); });
    tileInteractionSystem = std::make_unique<TileInteractionSystem>();

    tileSet = std::make_unique<Texture2D>("../assets/textures/tile_set.png");
    tileSetShader.initByShaderFile("../assets/shaders/tile_set.vs", "../assets/shaders/tile_set.fs");
    tileSetSpriteRenderer = std::make_unique<SpriteRenderer>(tileSetShader);
    tileMapRenderer = std::make_unique<TileMapRenderer>(*tileSet.get(), *tileSetSpriteRenderer.get());
    playerTexture = std::make_unique<Texture2D>("../assets/textures/player.png");
    screenTransitionShader.initByShaderFile("../assets/shaders/transition.vs", "../assets/shaders/transition.fs");
    screenTransition = std::make_unique<ScreenTransition>(screenTransitionShader);
    debugRenderer = std::make_unique<DebugRenderer>(window, screenWidth, screenHeight, gameData.debugRendererData);
    debugRenderer->onPlay.connect([this]
                                  { play(); });
    debugRenderer->onStep.connect([this]
                                  { step(); });

    luaScriptSystem->bindGameObjects(this, camera.get(), tileMap.get(), player.get(), screenTransition.get());

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

    tileSetSpriteRenderer->drawWithUV(
        *playerTexture.get(),
        projection,
        player->getPosition(),
        player->getSize(),
        player->getCurrentAnimation().getUVStart(),
        player->getCurrentAnimation().getUVEnd(),
        player->facingLeft());

    screenTransition->draw();

    debugRenderer->draw(*camera.get(), *tileMap.get(), *player.get());
}

void Game::resize(int screenWidth, int screenHeight)
{
    camera->resize(screenWidth, screenHeight);
    debugRenderer->resize(screenWidth, screenHeight);
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
        debugRenderer->update(deltaTime);

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

void Game::initGLFW()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(gameData.screenWidth, gameData.screenHeight, "platformer", NULL, NULL);
    if (!window)
    {
        throw std::runtime_error("Failed to create window");
    }

    glfwMakeContextCurrent(window);

    glfwSwapInterval(1);
}

void Game::initGameData()
{
    auto ec = glz::read_file_json(gameData, "../assets/game_data.json", std::string{});
    if (ec)
    {
        throw std::runtime_error("Failed to read game data json file");
    }
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

void Game::loadNextLevel()
{
    tileMap = std::make_unique<TileMap>((tileMap->getNextLevel()));
    luaScriptSystem->rebindTileMap(tileMap.get());
    camera->setWorldBounds(glm::vec2(0), glm::vec2(tileMap->getWorldWidth(), tileMap->getWorldHeight()));
    onLevelCompleteConnection = player->onLevelComplete.connect([this]()
                                                                {
        onLevelCompleteConnection.disconnect();
        luaScriptSystem->triggerLevelComplete(); });
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