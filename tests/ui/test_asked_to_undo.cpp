#include <catch2/catch_test_macros.hpp>
#include <array>
#include <imgui.h>
#include "helpers/headless_imgui.hpp"
#include "ui/asked_to_undo.hpp"

namespace
{
    void holdingDown(ImGuiKey chord)
    {
        ImGuiIO &io = ImGui::GetIO();
        io.AddKeyEvent(chord, true);
        io.AddKeyEvent(ImGuiKey_Z, true);
    }

    bool askedDuringAFrame(HeadlessImGui &gui)
    {
        bool asked = false;
        gui.frame([&] { asked = askedToUndo(); });
        return asked;
    }
}

TEST_CASE("Nobody asks to undo while the keys are up", "[AskedToUndo]")
{
    HeadlessImGui gui;

    REQUIRE_FALSE(askedDuringAFrame(gui));
}

TEST_CASE("Ctrl and z together ask to undo", "[AskedToUndo]")
{
    HeadlessImGui gui;
    holdingDown(ImGuiMod_Ctrl);

    REQUIRE(askedDuringAFrame(gui));
}

TEST_CASE("Cmd and z together ask to undo", "[AskedToUndo]")
{
    HeadlessImGui gui;
    holdingDown(ImGuiMod_Super);

    REQUIRE(askedDuringAFrame(gui));
}

TEST_CASE("Holding the keys down asks once, not every frame", "[AskedToUndo]")
{
    HeadlessImGui gui;
    holdingDown(ImGuiMod_Ctrl);

    REQUIRE(askedDuringAFrame(gui));
    REQUIRE_FALSE(askedDuringAFrame(gui));
}

TEST_CASE("Z on its own is a letter, not an undo", "[AskedToUndo]")
{
    HeadlessImGui gui;
    ImGui::GetIO().AddKeyEvent(ImGuiKey_Z, true);

    REQUIRE_FALSE(askedDuringAFrame(gui));
}

TEST_CASE("A text field being typed into keeps ctrl and z for itself", "[AskedToUndo]")
{
    HeadlessImGui gui;
    std::array<char, 32> typed{};
    auto drawing = [&] { ImGui::InputText("field", typed.data(), typed.size()); };
    gui.type("field", "hello", drawing);

    holdingDown(ImGuiMod_Ctrl);

    bool asked = true;
    gui.frame(
        [&]
        {
            drawing();
            asked = askedToUndo();
        });

    REQUIRE_FALSE(asked);
}
