#pragma once

#include "rendering/ui/editor_section.hpp"

class ImGuiManager;

class EditorUi
{
public:
    bool begin(const ImGuiManager &imGuiManager, bool showEditors);
    void end();
    EditorSection getSection() const;

private:
    EditorSection section = EditorSection::Playback;
};
