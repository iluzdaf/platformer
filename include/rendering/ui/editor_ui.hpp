#pragma once

#include "rendering/ui/editor_section.hpp"

class ImGuiManager;

class EditorUi
{
public:
    void draw(const ImGuiManager &imGuiManager, bool showEditors);
    EditorSection getSection() const;

    static void beginInspector(const ImGuiManager &imGuiManager, const char *title);
    static void endInspector();

private:
    EditorSection section = EditorSection::Playback;
};
