#include <string>
#include <catch2/catch_test_macros.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include "assets/texture_path_data.hpp"
#include "game/level_path_data.hpp"
#include "helpers/headless_imgui.hpp"
#include "scripting/script_path_data.hpp"
#include "ui/data_inspector.hpp"
#include "ui/inspector_fields.hpp"

namespace
{
    template <class Path> float drawnHeightOf(HeadlessImGui &gui, Path &path)
    {
        float reached = 0.0f;
        gui.frame(
            [&]
            {
                inspector::draw("path", path);
                reached = ImGui::GetCurrentWindow()->DC.CursorPos.y;
            });

        return reached;
    }
}

TEST_CASE("A path to a file draws itself rather than falling through", "[PathFields]")
{
    STATIC_REQUIRE(inspector::HasCustomField<TexturePathData>);
    STATIC_REQUIRE(inspector::HasCustomField<ScriptPathData>);
    STATIC_REQUIRE(inspector::HasCustomField<LevelPathData>);
}

TEST_CASE("A path naming no file, or one nobody has, says so beneath itself", "[PathFields]")
{
    HeadlessImGui gui;
    ScriptPathData onDisk{"scripts/npcs/rat.lua"};
    ScriptPathData nowhere{"scripts/npcs/dragon.lua"};
    ScriptPathData unnamed;

    float found = drawnHeightOf(gui, onDisk);

    REQUIRE(drawnHeightOf(gui, nowhere) > found);
    REQUIRE(drawnHeightOf(gui, unnamed) > found);
}

TEST_CASE("A path keeps what it was given after being drawn", "[PathFields]")
{
    HeadlessImGui gui;
    TexturePathData texture{"textures/somewhere.png"};
    LevelPathData level{"levels/level6.json"};

    drawnHeightOf(gui, texture);
    drawnHeightOf(gui, level);

    REQUIRE(texture.path == "textures/somewhere.png");
    REQUIRE(level.path == "levels/level6.json");
}

TEST_CASE("A path is read and written as the string it holds", "[PathFields]")
{
    ScriptPathData script;
    REQUIRE_FALSE(glz::read_json(script, "\"scripts/player.lua\""));
    REQUIRE(script.path == "scripts/player.lua");

    std::string json;
    REQUIRE_FALSE(glz::write_json(LevelPathData{"levels/level1.json"}, json));
    REQUIRE(json == "\"levels/level1.json\"");
}
