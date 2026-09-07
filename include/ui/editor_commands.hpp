#pragma once

#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include "ui/editor_command.hpp"
#include "game/level_data.hpp"

struct EditorCommands
{
    EditorCommand<> onPlay, onPause, onStep;
    EditorCommand<const LevelData &> onLevelEdited;
    EditorCommand<const LevelData &, const glm::vec2 &> onLevelResized;
    EditorCommand<> onSettingsChanged, onCameraChanged;
    EditorCommand<const std::string &> onLoadLevel, onWarmTexture;

    void drain()
    {
        onPlay.drain();
        onPause.drain();
        onStep.drain();
        onLevelEdited.drain();
        onLevelResized.drain();
        onSettingsChanged.drain();
        onCameraChanged.drain();
        onLoadLevel.drain();
        onWarmTexture.drain();
    }
};
