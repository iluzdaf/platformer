#include <imgui.h>
#include "ui/asked_to_undo.hpp"

bool askedToUndo()
{
    if (ImGui::GetIO().WantTextInput)
        return false;

    return ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_Z) ||
           ImGui::IsKeyChordPressed(ImGuiMod_Super | ImGuiKey_Z);
}
