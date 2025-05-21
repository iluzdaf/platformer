#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include "texture2d.hpp"
#include "shader.hpp"
#include "sprite_renderer.hpp"

const unsigned int screenWidth = 800;
const unsigned int screenHeight = 600;
float xPos = 0.0f, yPos = 0.0f;

void processInput(GLFWwindow *window, float deltaTime)
{
    float speed = 2.0f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        yPos += speed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        yPos -= speed;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        xPos -= speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        xPos += speed;
}

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

    float lastTime = glfwGetTime();

    try
    {
        glm::mat4 projection = glm::ortho(0.0f, (float)screenWidth, (float)screenHeight, 0.0f);
        Texture2D playerTexture("../textures/player.png");
        Shader spriteShader;
        spriteShader.initByPath("../shaders/sprite.vs", "../shaders/sprite.fs");
        SpriteRenderer spriteRenderer;

        while (!glfwWindowShouldClose(window))
        {
            float currentTime = glfwGetTime();
            float deltaTime = currentTime - lastTime;
            lastTime = currentTime;

            processInput(window, deltaTime);

            glClearColor(0.1f, 0.12f, 0.15f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            spriteRenderer.draw(playerTexture, spriteShader, projection, glm::vec2(100, 100), glm::vec2(32, 32));

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