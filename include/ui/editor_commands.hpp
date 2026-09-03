#pragma once

#include <string>
#include "ui/editor_command.hpp"
#include "game/level_data.hpp"

struct EditorCommands
{
    EditorCommand<> onPlay, onPause, onStep, onRespawn;
    EditorCommand<const LevelData &> onLevelEdited;
    EditorCommand<> onSettingsChanged, onCameraChanged;
    EditorCommand<const std::string &> onLoadLevel, onWarmTexture;

    void drain()
    {
        onPlay.drain();
        onPause.drain();
        onStep.drain();
        onRespawn.drain();
        onLevelEdited.drain();
        onSettingsChanged.drain();
        onCameraChanged.drain();
        onLoadLevel.drain();
        onWarmTexture.drain();
    }
};
