#include <optional>
#include <imgui.h>
#include "ui/section_mark.hpp"
#include "ui/unsaved_colours.hpp"

std::optional<ImVec4> markFor(bool unsaved, bool cannotSave)
{
    if (cannotSave)
        return CannotSaveColour;

    if (unsaved)
        return UnsavedColour;

    return std::nullopt;
}
