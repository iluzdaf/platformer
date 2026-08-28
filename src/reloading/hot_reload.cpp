#include <functional>
#include <iostream>
#include "reloading/hot_reload.hpp"
#include "game/game.hpp"

namespace
{
    // A broken asset saved mid edit should say so and leave the game running.
    void reporting(const std::function<void()> &rebuild)
    {
        try
        {
            rebuild();
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << std::endl;
        }
    }
}

HotReload::HotReload(Game &game) : game(game)
{
    levelWatcher.onLevelChanged.connect(
        [this](const std::string &levelPath)
        {
            if (this->game.isPlaying(levelPath))
                reporting([&] { this->game.loadLevel(levelPath); });
        });

    assetWatcher.onShaderChanged.connect(
        [this](const std::string &shaderPath)
        { reporting([&] { this->game.reloadShader(shaderPath); }); });

    assetWatcher.onTextureChanged.connect(
        [this](const std::string &texturePath)
        { reporting([&] { this->game.reloadTexture(texturePath); }); });

    gameDataWatcher.onGameDataChanged.connect([this] { reporting([&] { this->game.reload(); }); });

    scriptWatcher.onScriptsChanged.connect([this]
                                           { reporting([&] { this->game.reloadScripts(); }); });
}

void HotReload::process()
{
    levelWatcher.process();
    assetWatcher.process();
    gameDataWatcher.process();
    scriptWatcher.process();
}
