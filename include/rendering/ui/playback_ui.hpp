#pragma once

struct EditorCommands;

class PlaybackUi
{
public:
    void draw(bool paused, EditorCommands &commands) const;
};
