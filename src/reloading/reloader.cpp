#include <exception>
#include <functional>
#include <iostream>
#include <string>
#include <string_view>
#include "reloading/reloader.hpp"
#include "assets/asset_paths.hpp"

namespace
{
    bool under(std::string_view path, std::string_view directory)
    {
        return path.size() > directory.size() && path.starts_with(directory) &&
               path[directory.size()] == '/';
    }

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

void Reloader::levelLoaded(const std::string &levelPath)
{
    playing = levelPath;
}

void Reloader::fileChanged(const std::string &path)
{
    if (under(path, assets::Levels) && path.ends_with(".json"))
        levelChanged(path);
    else if (under(path, assets::Textures) && path.ends_with(".png"))
        textureChanged(path);
    else if (under(path, assets::Shaders) && (path.ends_with(".vs") || path.ends_with(".fs")))
        shaderChanged(path);
    else if (under(path, assets::Scripts))
        scriptsChanged();
    else if (under(path, assets::Data))
        gameDataChanged();
}

void Reloader::levelChanged(const std::string &levelPath)
{
    if (levelPath != playing)
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
