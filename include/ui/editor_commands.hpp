#pragma once

#include <string>
#include "ui/editor_command.hpp"

struct EditorCommands
{
    EditorCommand<> onPlay, onPause, onStep, onRespawn, onNpcsChanged;
    EditorCommand<> onSettingsChanged, onCameraChanged;
    EditorCommand<const std::string &> onLoadLevel;

    void drain()
    {
        onPlay.drain();
        onPause.drain();
        onStep.drain();
        onRespawn.drain();
        onNpcsChanged.drain();
        onSettingsChanged.drain();
        onCameraChanged.drain();
        onLoadLevel.drain();
    }
};
