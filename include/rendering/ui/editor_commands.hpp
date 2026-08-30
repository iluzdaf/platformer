#pragma once

#include <string>
#include <signals.hpp>

struct EditorCommands
{
    fteng::signal<void()> onPlay, onPause, onStep, onRespawn;
    fteng::signal<void()> onSettingsChanged, onCameraChanged;
    fteng::signal<void(const std::string &)> onLoadLevel;
};
