#include <catch2/catch_test_macros.hpp>
#include <array>
#include <cstddef>
#include <optional>
#include <imgui.h>
#include "game/level_resizing.hpp"
#include "test_helpers/headless_imgui.hpp"
#include "ui/size_buttons.hpp"

namespace
{
    struct Row
    {
        ImVec2 smaller, larger;
    };

    std::array<Row, 4> whereTheButtonsAre(HeadlessImGui &gui)
    {
        std::array<Row, 4> rows;
        auto measure = [&]
        {
            ImVec2 top = ImGui::GetCursorScreenPos();
            const ImGuiStyle &style = ImGui::GetStyle();
            float line = ImGui::GetTextLineHeight();
            float buttonWidth = ImGui::CalcTextSize("-").x + style.FramePadding.x * 2.0f;
            float minusMiddle = ImGui::GetWindowPos().x + SizeLabelWidth + buttonWidth * 0.5f;
            float plusMiddle = minusMiddle + buttonWidth + style.ItemSpacing.x;
            for (int row = 0; row < 4; ++row)
            {
                float middle = top.y + ImGui::GetTextLineHeightWithSpacing() +
                               static_cast<float>(row) * (line + style.ItemSpacing.y) + line * 0.5f;
                rows[static_cast<std::size_t>(row)] = {
                    ImVec2(minusMiddle, middle), ImVec2(plusMiddle, middle)};
            }

            drawSizeButtons(3, 2);
        };
        gui.frame(measure);
        gui.frame(measure);
        return rows;
    }

    std::optional<Resize> askedByClicking(HeadlessImGui &gui, ImVec2 at)
    {
        std::optional<Resize> asked;
        gui.clickAt(
            at,
            [&]
            {
                if (std::optional<Resize> resize = drawSizeButtons(3, 2))
                    asked = resize;
            });
        return asked;
    }
}

TEST_CASE("Left alone, the size buttons ask for nothing", "[SizeButtons]")
{
    HeadlessImGui gui;
    std::optional<Resize> asked;

    gui.frame([&] { asked = drawSizeButtons(3, 2); });

    REQUIRE_FALSE(asked.has_value());
}

TEST_CASE("Each side's plus asks for a larger map on that side", "[SizeButtons]")
{
    HeadlessImGui gui;
    std::array<Row, 4> rows = whereTheButtonsAre(gui);

    REQUIRE(askedByClicking(gui, rows[0].larger) == Resize{Side::Left, true});
    REQUIRE(askedByClicking(gui, rows[1].larger) == Resize{Side::Right, true});
    REQUIRE(askedByClicking(gui, rows[2].larger) == Resize{Side::Above, true});
    REQUIRE(askedByClicking(gui, rows[3].larger) == Resize{Side::Below, true});
}

TEST_CASE("Each side's minus asks for a smaller map on that side", "[SizeButtons]")
{
    HeadlessImGui gui;
    std::array<Row, 4> rows = whereTheButtonsAre(gui);

    REQUIRE(askedByClicking(gui, rows[0].smaller) == Resize{Side::Left, false});
    REQUIRE(askedByClicking(gui, rows[1].smaller) == Resize{Side::Right, false});
    REQUIRE(askedByClicking(gui, rows[2].smaller) == Resize{Side::Above, false});
    REQUIRE(askedByClicking(gui, rows[3].smaller) == Resize{Side::Below, false});
}
