#include <catch2/catch_test_macros.hpp>
#include <optional>
#include "ui/section_mark.hpp"
#include "ui/unsaved_colours.hpp"

namespace
{
    bool same(const ImVec4 &one, const ImVec4 &other)
    {
        return one.x == other.x && one.y == other.y && one.z == other.z && one.w == other.w;
    }
}

TEST_CASE("A section with nothing to say is not marked", "[SectionMark]")
{
    REQUIRE_FALSE(markFor(false, false));
}

TEST_CASE("A section with something unsaved is marked unsaved", "[SectionMark]")
{
    std::optional<ImVec4> mark = markFor(true, false);

    REQUIRE(mark);
    REQUIRE(same(*mark, UnsavedColour));
}

TEST_CASE("A section that cannot save says so instead", "[SectionMark]")
{
    std::optional<ImVec4> mark = markFor(true, true);

    REQUIRE(mark);
    REQUIRE(same(*mark, CannotSaveColour));
}

TEST_CASE("A section that cannot save is marked even with nothing unsaved", "[SectionMark]")
{
    std::optional<ImVec4> mark = markFor(false, true);

    REQUIRE(mark);
    REQUIRE(same(*mark, CannotSaveColour));
}
