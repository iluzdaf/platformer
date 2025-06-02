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

    int screenWidth, screenHeight;
    glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
    camera = std::make_unique<Camera2D>(gameData.cameraData, screenWidth, screenHeight);
    keyboardManager.registerKey(GLFW_KEY_UP);
    keyboardManager.registerKey(GLFW_KEY_LEFT);
    keyboardManager.registerKey(GLFW_KEY_RIGHT);
    keyboardManager.registerKey(GLFW_KEY_RIGHT_SHIFT);

    tileMap = std::make_unique<TileMap>(("../assets/tile_maps/level1.json"));
    player = std::make_unique<Player>(gameData.playerData, gameData.physicsData);
    player->setPosition(tileMap->getPlayerStartWorldPosition());
    player->onLevelComplete.connect([this]()
                                    { this->loadNextLevel(); });
    camera->setWorldBounds(glm::vec2(0), glm::vec2(tileMap->getWorldWidth(), tileMap->getWorldHeight()));

    tileSet = std::make_unique<Texture2D>("../assets/textures/tile_set.png");
    tileSetShader.initByShaderFile("../assets/shaders/tile_set.vs", "../assets/shaders/tile_set.fs");
    tileSetSpriteRenderer = std::make_unique<SpriteRenderer>(tileSetShader);
    tileMapRenderer = std::make_unique<TileMapRenderer>(*tileSet.get(), *tileSetSpriteRenderer.get());
    playerTexture = std::make_unique<Texture2D>("../assets/textures/player.png");
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
}

void Game::update(float deltaTime)
{
    player->update(deltaTime, *tileMap.get());
    camera->follow(player->getPosition());
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
}

void Game::resize(int screenWidth, int screenHeight)
{
    camera->resize(screenWidth, screenHeight);
}

void Game::run()
{
    float lastTime = glfwGetTime();
    while (!glfwWindowShouldClose(window))
    {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        preFixedUpdate();

        timestepper.run(deltaTime, [&](float dt)
                        { fixedUpdate(dt); });
        update(deltaTime);

        if (onEndOfFrame)
        {
            onEndOfFrame();
            onEndOfFrame = nullptr;
        }

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
    keyboardManager.update(window);
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
    onEndOfFrame = [this]()
    {
        tileMap = std::make_unique<TileMap>((tileMap->getNextLevel()));
        player->setPosition(tileMap->getPlayerStartWorldPosition());
        camera->setWorldBounds(glm::vec2(0), glm::vec2(tileMap->getWorldWidth(), tileMap->getWorldHeight()));
    };
}