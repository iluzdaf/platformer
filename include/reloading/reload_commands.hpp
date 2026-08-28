#pragma once

#include <functional>
#include <string>
#include <signals.hpp>

struct ReloadCommands
{
    std::function<bool(const std::string &)> isPlaying;

    fteng::signal<void(const std::string &)> onLoadLevel, onReloadShader, onReloadTexture;
    fteng::signal<void()> onReload, onReloadScripts;
};
