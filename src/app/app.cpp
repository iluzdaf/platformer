#include <chrono>
#include "app/app.hpp"
#include <string>
#include "game/game_data.hpp"
#include "reloading/reload_commands.hpp"

App::App()
    : window(GameSettingsData().windowWidth, GameSettingsData().windowHeight, "platformer"),
      game(window)
{
    ReloadCommands &commands = hotReload.commands();
    commands.isPlaying = [this](const std::string &levelPath) { return game.isPlaying(levelPath); };
    commands.onLoadLevel.connect([this](const std::string &levelPath)
                                 { game.loadLevel(levelPath); });
    commands.onReloadShader.connect([this](const std::string &shaderPath)
                                    { game.reloadShader(shaderPath); });
    commands.onReloadTexture.connect([this](const std::string &texturePath)
                                     { game.reloadTexture(texturePath); });
    commands.onReload.connect([this] { game.reload(); });
    commands.onReloadScripts.connect([this] { game.reloadScripts(); });
}

void App::run()
{
    auto lastTime = std::chrono::steady_clock::now();
    while (!window.shouldClose())
    {
        auto currentTime = std::chrono::steady_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        hotReload.process();
        game.frame(deltaTime);

        window.swapBuffers();
        window.pollEvents();
    }
}
