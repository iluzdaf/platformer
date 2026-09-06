#include <optional>
#include <utility>
#include <imgui.h>
#include "ui/size_buttons.hpp"
#include "game/level_resizing.hpp"

namespace
{
    std::optional<Resize> drawSideRow(const char *label, Side side)
    {
        std::optional<Resize> asked;
        ImGui::PushID(label);
        ImGui::TextUnformatted(label);
        ImGui::SameLine(SizeLabelWidth);
        if (ImGui::SmallButton("-"))
            asked = Resize{side, false};
        ImGui::SameLine();
        if (ImGui::SmallButton("+"))
            asked = Resize{side, true};
        ImGui::PopID();
        return asked;
    }
}

std::optional<Resize> drawSizeButtons(int width, int height)
{
    ImGui::Text("%d by %d tiles", width, height);

    std::optional<Resize> asked;
    for (const auto &[label, side] :
         {std::pair{"column left", Side::Left},
          std::pair{"column right", Side::Right},
          std::pair{"row above", Side::Above},
          std::pair{"row below", Side::Below}})
        if (std::optional<Resize> row = drawSideRow(label, side))
            asked = row;

    return asked;
}
