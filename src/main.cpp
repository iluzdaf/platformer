#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include "rendering/texture2d.hpp"
#include "rendering/shader.hpp"
#include "rendering/sprite_renderer.hpp"
#include "rendering/tile_map_renderer.hpp"
#include "game/tile_map.hpp"
#include "game/player.hpp"
#include "game/fixed_time_step.hpp"

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

    try
    {
        float lastTime = glfwGetTime();

        glm::mat4 projection = glm::ortho(0.0f, (float)screenWidth, (float)screenHeight, 0.0f);

        TileMap tileMap(50, 38);
        tileMap.setTile(0, 37, 31);
        for (int i = 1; i < 49; ++i)
        {
            tileMap.setTile(i, 37, 32);
        }
        tileMap.setTile(49, 37, 33);

        tileMap.setTile(25, 32, 31);
        tileMap.setTile(26, 32, 32);
        tileMap.setTile(27, 32, 33);

        tileMap.setTile(15, 34, 26);
        tileMap.setTile(15, 35, 22);
        tileMap.setTile(15, 36, 22);
        tileMap.setTile(15, 37, 20);

        tileMap.setTileTypes({
            {31, TileKind::Solid},
            {32, TileKind::Solid},
            {33, TileKind::Solid},
            {26, TileKind::Solid},
            {20, TileKind::Solid},
            {22, TileKind::Solid},
        });
        Texture2D tileSet("../textures/tile_set.png");
        Shader tileSetShader;
        tileSetShader.initByPath("../shaders/tile_set.vs", "../shaders/tile_set.fs");
        SpriteRenderer tileSetSpriteRenderer(tileSetShader);
        TileMapRenderer tileMapRenderer(tileSet, tileSetSpriteRenderer);

        Texture2D playerTexture("../textures/player.png");
        Player player(glm::vec2(100, 100));

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
                            { player.update(dt, tileMap); });

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