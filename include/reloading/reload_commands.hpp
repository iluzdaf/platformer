#pragma once

#include <string>
#include <signals.hpp>

struct ReloadCommands
{
    fteng::signal<void(const std::string &)> onLoadLevel, onReloadShader, onReloadTexture;
    fteng::signal<void()> onReload, onReloadScripts;
};
