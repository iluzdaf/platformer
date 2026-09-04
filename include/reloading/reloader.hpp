#pragma once

#include <string>
#include "reloading/reload_commands.hpp"

class Reloader
{
public:
    ReloadCommands commands;

    void levelLoaded(const std::string &levelPath);

    void fileChanged(const std::string &path);

    void levelChanged(const std::string &levelPath);
    void shaderChanged(const std::string &shaderPath);
    void textureChanged(const std::string &texturePath);
    void gameDataChanged();
    void scriptsChanged();

private:
    std::string playing;
};
