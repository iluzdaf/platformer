#include <chrono>
#include "app/app.hpp"
#include "game/game_data.hpp"

App::App() : window(GameData().windowWidth, GameData().windowHeight, "platformer"), game(window)
{
}

void App::run()
{
    auto lastTime = std::chrono::steady_clock::now();
    while (!window.shouldClose())
    {
        auto currentTime = std::chrono::steady_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        game.frame(deltaTime);

        window.swapBuffers();
        window.pollEvents();
    }
}
