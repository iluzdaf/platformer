#include <exception>
#include <functional>
#include <iostream>
#include <string>
#include "reloading/reloader.hpp"

namespace
{
    void reporting(const std::function<void()> &rebuild)
    {
        try
        {
            rebuild();
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }
    }
}

void Reloader::levelChanged(const std::string &levelPath)
{
    if (commands.isPlaying && !commands.isPlaying(levelPath))
        return;

    reporting([&] { commands.onLoadLevel(levelPath); });
}

void Reloader::shaderChanged(const std::string &shaderPath)
{
    reporting([&] { commands.onReloadShader(shaderPath); });
}

void Reloader::textureChanged(const std::string &texturePath)
{
    reporting([&] { commands.onReloadTexture(texturePath); });
}

void Reloader::gameDataChanged()
{
    reporting([&] { commands.onReload(); });
}

void Reloader::scriptsChanged()
{
    reporting([&] { commands.onReloadScripts(); });
}
