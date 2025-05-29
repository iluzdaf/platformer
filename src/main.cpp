#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glaze/glaze.hpp>
#include "rendering/texture2d.hpp"
#include "rendering/shader.hpp"
#include "rendering/sprite_renderer.hpp"
#include "rendering/tile_map_renderer.hpp"
#include "game/tile_map/tile_map.hpp"
#include "game/player/player.hpp"
#include "game/fixed_time_step.hpp"
#include "game/camera2d.hpp"
#include "game/game_data.hpp"
#include "input/keyboard_manager.hpp"

int main()
{
    try
    {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        GameData gameData;
        auto ec = glz::read_file_json(gameData, "../assets/game_data.json", std::string{});
        if (ec)
        {
            throw std::runtime_error("Failed to read game data json file");
        }

        GLFWwindow *window = glfwCreateWindow(gameData.cameraData.width, gameData.cameraData.height, "Platformer", NULL, NULL);
        if (!window)
        {
            throw std::runtime_error("Failed to create window");
        }
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            throw std::runtime_error("Failed to initialize GLAD");
        }
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        float lastTime = glfwGetTime();

        TileMap tileMap("../assets/tile_maps/level1.json");
        Texture2D tileSet("../assets/textures/tile_set.png");
        Shader tileSetShader;
        tileSetShader.initByShaderFile("../assets/shaders/tile_set.vs", "../assets/shaders/tile_set.fs");
        SpriteRenderer tileSetSpriteRenderer(tileSetShader);
        TileMapRenderer tileMapRenderer(tileSet, tileSetSpriteRenderer);

        Camera2D camera(gameData.cameraData);
        camera.setWorldBounds(glm::vec2(0), glm::vec2(tileMap.getWorldWidth(), tileMap.getWorldHeight()));

        Texture2D playerTexture("../assets/textures/player.png");
        Player player(gameData.playerData, gameData.physicsData);

        KeyboardManager keyboardManager;
        keyboardManager.registerKey(GLFW_KEY_UP);
        keyboardManager.registerKey(GLFW_KEY_LEFT);
        keyboardManager.registerKey(GLFW_KEY_RIGHT);

        while (!glfwWindowShouldClose(window))
        {
            float currentTime = glfwGetTime();
            float deltaTime = currentTime - lastTime;
            lastTime = currentTime;

            glClearColor(0.1f, 0.12f, 0.15f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            keyboardManager.update(window);
            if (keyboardManager.isPressed(GLFW_KEY_UP))
            {
                player.jump();
            }
            if (keyboardManager.isDown(GLFW_KEY_LEFT))
            {
                player.moveLeft();
            }
            if (keyboardManager.isDown(GLFW_KEY_RIGHT))
            {
                player.moveRight();
            }

            FixedTimeStep timestepper(0.01f);
            timestepper.run(deltaTime, [&](float dt)
                            { player.fixedUpdate(dt, tileMap); });
            player.update(deltaTime, tileMap);
            camera.follow(player.getPosition());
            tileMap.update(deltaTime);

            glm::mat4 projection = camera.getProjection();
            tileMapRenderer.draw(tileMap, projection);
            tileSetSpriteRenderer.drawWithUV(
                playerTexture,
                projection,
                player.getPosition(),
                player.getSize(),
                player.getCurrentAnimation().getUVStart(),
                player.getCurrentAnimation().getUVEnd(),
                player.isFacingLeft());

            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        glfwTerminate();
        return 0;
    }
    catch (const std::runtime_error &e)
    {
        std::cout << e.what() << "\n";
        glfwTerminate();
        return -1;
    }
}