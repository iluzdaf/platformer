#include <catch2/catch_test_macros.hpp>

#ifndef SKIP_OPENGL_TESTS
#include "rendering/texture2d.hpp"
#include "test_helpers/made_sheet.hpp"

TEST_CASE("A made sheet is the shape it was asked for", "[MadeSheet]")
{
    Texture2D sheet = aSheetOf(3, 2);

    REQUIRE(sheet.getWidth() == 48);
    REQUIRE(sheet.getHeight() == 32);
}

TEST_CASE("A made sheet may have cells of any size", "[MadeSheet]")
{
    Texture2D sheet = aSheetOf(5, 1, 8);

    REQUIRE(sheet.getWidth() == 40);
    REQUIRE(sheet.getHeight() == 8);
}
#endif
