#include <string>
#include "reloading/hot_reload.hpp"

HotReload::HotReload()
{
    levelWatcher.onLevelChanged.connect([this](const std::string &levelPath)
                                        { reloader.levelChanged(levelPath); });

    assetWatcher.onShaderChanged.connect([this](const std::string &shaderPath)
                                         { reloader.shaderChanged(shaderPath); });

    assetWatcher.onTextureChanged.connect([this](const std::string &texturePath)
                                          { reloader.textureChanged(texturePath); });

    gameDataWatcher.onGameDataChanged.connect([this] { reloader.gameDataChanged(); });

    scriptWatcher.onScriptsChanged.connect([this] { reloader.scriptsChanged(); });
}

void HotReload::process()
{
    levelWatcher.process();
    assetWatcher.process();
    gameDataWatcher.process();
    scriptWatcher.process();
}

Reloader &HotReload::getReloader()
{
    return reloader;
}
