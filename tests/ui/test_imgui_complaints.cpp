#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <tuple>
#include <imgui.h>
#include "helpers/headless_imgui.hpp"

TEST_CASE("A frame that draws two items with one id says which", "[ImGuiComplaints]")
{
    HeadlessImGui gui;

    REQUIRE_THROWS_WITH(
        gui.frame(
            [&]
            {
                std::ignore = ImGui::Button("twice");
                std::ignore = ImGui::Button("twice");
            }),
        Catch::Matchers::ContainsSubstring("same id") &&
            Catch::Matchers::ContainsSubstring("twice"));
}

TEST_CASE("The same item drawn in frame after frame says nothing", "[ImGuiComplaints]")
{
    HeadlessImGui gui;
    auto drawing = [&] { std::ignore = ImGui::Button("once"); };

    REQUIRE_NOTHROW(gui.frame(drawing));
    REQUIRE_NOTHROW(gui.frame(drawing));
}

TEST_CASE("An id left on the stack is an error ImGui hands back", "[ImGuiComplaints]")
{
    HeadlessImGui gui;

    REQUIRE_THROWS_WITH(
        gui.frame([&] { ImGui::PushID("forgotten"); }),
        Catch::Matchers::ContainsSubstring("imgui"));
}

TEST_CASE("Items told apart by the id they sit under say nothing", "[ImGuiComplaints]")
{
    HeadlessImGui gui;

    REQUIRE_NOTHROW(gui.frame(
        [&]
        {
            ImGui::PushID("here");
            std::ignore = ImGui::Button("twice");
            ImGui::PopID();

            ImGui::PushID("there");
            std::ignore = ImGui::Button("twice");
            ImGui::PopID();
        }));
}
