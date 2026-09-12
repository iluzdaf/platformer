#include <catch2/catch_test_macros.hpp>
#include <string>
#include <tuple>
#include <imgui.h>
#include "assets/texture_path_data.hpp"
#include "helpers/colours_drawn.hpp"
#include "helpers/headless_imgui.hpp"
#include "serialization/only_what_differs.hpp"
#include "ui/data_inspector.hpp"
#include "ui/saved_in_scope.hpp"
#include "ui/unsaved_colours.hpp"

namespace marks
{
    struct SomewhereData
    {
        TexturePathData texture;
    };
}

namespace
{
    const std::string Here = "textures/coin.png";
    const std::string There = "textures/heart.png";
    const std::string Gone = "textures/nowhere.png";

    template <class Draw> bool redIn(HeadlessImGui &gui, Draw &&draw)
    {
        return drawnIn(gui, CannotSaveColour, draw);
    }

    template <class Draw> bool yellowIn(HeadlessImGui &gui, Draw &&draw)
    {
        return drawnIn(gui, UnsavedColour, draw);
    }

    auto drawing(const marks::SomewhereData &saved, marks::SomewhereData &now)
    {
        return [&saved, &now]
        {
            SavedInScope was(differs::compact(saved));
            std::ignore = inspector::draw("texture", now.texture);
        };
    }
}

TEST_CASE("A field naming a file that is not there says so in red", "[FieldMarks]")
{
    HeadlessImGui gui;
    marks::SomewhereData saved{Gone};
    marks::SomewhereData now{Gone};

    REQUIRE(redIn(gui, drawing(saved, now)));
}

TEST_CASE("A field naming a file that is there says nothing in red", "[FieldMarks]")
{
    HeadlessImGui gui;
    marks::SomewhereData saved{Here};
    marks::SomewhereData now{Here};

    REQUIRE_FALSE(redIn(gui, drawing(saved, now)));
}

TEST_CASE("A field edited away from what was saved says so in yellow", "[FieldMarks]")
{
    HeadlessImGui gui;
    marks::SomewhereData saved{Here};
    marks::SomewhereData now{Here};

    REQUIRE_FALSE(yellowIn(gui, drawing(saved, now)));

    now.texture = There;
    REQUIRE(yellowIn(gui, drawing(saved, now)));
}

TEST_CASE("A field both edited and refused says the refusal, not the edit", "[FieldMarks]")
{
    HeadlessImGui gui;
    marks::SomewhereData saved{Here};
    marks::SomewhereData now{Gone};

    REQUIRE(redIn(gui, drawing(saved, now)));
    REQUIRE_FALSE(yellowIn(gui, drawing(saved, now)));
}
