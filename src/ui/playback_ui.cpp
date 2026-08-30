#include <imgui.h>
#include "ui/playback_ui.hpp"
#include "ui/editor_commands.hpp"

void PlaybackUi::draw(bool paused, EditorCommands &commands) const
{
    if (ImGui::Button(paused ? "play" : "pause", ImVec2(60.0f, 0.0f)))
    {
        if (paused)
            commands.onPlay();
        else
            commands.onPause();
    }

    ImGui::SameLine();
    if (ImGui::Button("step", ImVec2(60.0f, 0.0f)))
        commands.onStep();

    ImGui::TextDisabled("%s", paused ? "stopped" : "running");
}
