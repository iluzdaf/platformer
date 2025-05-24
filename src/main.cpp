#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <optional>
#include "rendering/texture2d.hpp"
#include "rendering/shader.hpp"
#include "rendering/sprite_renderer.hpp"
#include "rendering/tile_map_renderer.hpp"
#include "game/tile_map.hpp"
#include "game/player.hpp"
#include "game/fixed_time_step.hpp"
#include "game/camera2d.hpp"

const unsigned int screenWidth = 800;
const unsigned int screenHeight = 600;

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(screenWidth, screenHeight, "Platformer", NULL, NULL);
    if (!window)
    {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    try
    {
        float lastTime = glfwGetTime();

        TileMap tileMap(50, 38);
        tileMap.setTileIndex(0, 37, 28);
        for (int i = 1; i < 49; ++i)
        {
            tileMap.setTileIndex(i, 37, 29);
        }
        tileMap.setTileIndex(49, 37, 31);

        tileMap.setTileIndex(20, 34, 39);
        tileMap.setTileIndex(21, 34, 40);
        tileMap.setTileIndex(22, 34, 41);

        tileMap.setTileIndex(15, 34, 32);
        tileMap.setTileIndex(15, 35, 26);
        tileMap.setTileIndex(15, 36, 26);
        tileMap.setTileIndex(15, 37, 14);

        tileMap.setTileIndex(10, 36, 42);

        tileMap.setTiles({
            {28, {TileKind::Solid, std::nullopt}},
            {29, {TileKind::Solid, std::nullopt}},
            {31, {TileKind::Solid, std::nullopt}},
            {39, {TileKind::Solid, std::nullopt}},
            {40, {TileKind::Solid, std::nullopt}},
            {41, {TileKind::Solid, std::nullopt}},
            {32, {TileKind::Solid, std::nullopt}},
            {26, {TileKind::Solid, std::nullopt}},
            {14, {TileKind::Solid, std::nullopt}},
            {42, {TileKind::Empty, TileAnimation({42, 43, 44, 45, 46, 47, 48, 35, 36, 37}, 0.075f)}}
        });
        Texture2D tileSet("../textures/tile_set.png");
        Shader tileSetShader;
        tileSetShader.initByPath("../shaders/tile_set.vs", "../shaders/tile_set.fs");
        SpriteRenderer tileSetSpriteRenderer(tileSetShader);
        TileMapRenderer tileMapRenderer(tileSet, tileSetSpriteRenderer);

        glm::mat4 projection = glm::ortho(0.0f, (float)screenWidth, (float)screenHeight, 0.0f);
        Camera2D camera(screenWidth, screenHeight, 4);
        camera.setWorldBounds(glm::vec2(0), glm::vec2(tileMap.getWidth() * tileMap.getTileSize(), tileMap.getHeight() * tileMap.getTileSize()));

        Texture2D playerTexture("../textures/player.png");
        Player player(glm::vec2(100, 400));

        while (!glfwWindowShouldClose(window))
        {
            float currentTime = glfwGetTime();
            float deltaTime = currentTime - lastTime;
            lastTime = currentTime;

            glClearColor(0.1f, 0.12f, 0.15f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                player.moveLeft();
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                player.moveRight();
            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
                player.jump();

            FixedTimeStep timestepper(0.01f);
            timestepper.run(deltaTime, [&](float dt)
                            { player.update(dt, tileMap); 
                            camera.follow(player.getPosition()); });
            tileMap.update(deltaTime);

            glm::mat4 projection = camera.getProjection();
            tileMapRenderer.draw(tileMap, projection);
            tileSetSpriteRenderer.drawWithUV(
                playerTexture,
                projection,
                player.getPosition(),
                player.size,
                player.getCurrentAnimation().getUVStart(),
                player.getCurrentAnimation().getUVEnd(),
                player.isFacingLeft());

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }
    catch (const std::runtime_error &e)
    {
        std::cout << e.what() << "\n";
    }

    glfwTerminate();
    return 0;
}