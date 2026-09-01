#pragma once

#include <imgui.h>

namespace inspector
{
    struct Edited
    {
        bool whileEditing = false;
        bool onCommit = false;

        Edited &operator|=(const Edited &other)
        {
            whileEditing = whileEditing || other.whileEditing;
            onCommit = onCommit || other.onCommit;

            return *this;
        }

        explicit operator bool() const
        {
            return whileEditing || onCommit;
        }
    };

    inline Edited justEdited(bool changed)
    {
        return {changed, ImGui::IsItemDeactivatedAfterEdit()};
    }
}
