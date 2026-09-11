#include <catch2/catch_test_macros.hpp>
#include <optional>
#include <tuple>
#include <string>
#include "ui/edit_settling.hpp"

TEST_CASE("The first look at the data is nothing to undo", "[EditSettling]")
{
    EditSettling settling;

    REQUIRE_FALSE(settling.settled("as it is", false));
}

TEST_CASE("A change that is over says what the data was before it", "[EditSettling]")
{
    EditSettling settling;
    std::ignore = settling.settled("as it was", false);

    REQUIRE(settling.settled("as it is now", false) == std::optional<std::string>("as it was"));
}

TEST_CASE("A drag is one edit, said once it lets go", "[EditSettling]")
{
    EditSettling settling;
    std::ignore = settling.settled("40", false);

    REQUIRE_FALSE(settling.settled("41", true));
    REQUIRE_FALSE(settling.settled("42", true));
    REQUIRE_FALSE(settling.settled("43", true));

    REQUIRE(settling.settled("43", false) == std::optional<std::string>("40"));
}

TEST_CASE("Data left where it was is no edit at all", "[EditSettling]")
{
    EditSettling settling;
    std::ignore = settling.settled("40", false);

    REQUIRE_FALSE(settling.settled("41", true));
    REQUIRE_FALSE(settling.settled("40", true));
    REQUIRE_FALSE(settling.settled("40", false));
}

TEST_CASE("Nothing changing says nothing, frame after frame", "[EditSettling]")
{
    EditSettling settling;
    std::ignore = settling.settled("as it is", false);

    REQUIRE_FALSE(settling.settled("as it is", false));
    REQUIRE_FALSE(settling.settled("as it is", false));
}

TEST_CASE("Starting again forgets the edit that was under way", "[EditSettling]")
{
    EditSettling settling;
    std::ignore = settling.settled("40", false);
    REQUIRE_FALSE(settling.settled("41", true));

    settling.startsAgainFrom("41");

    REQUIRE_FALSE(settling.settled("41", false));
}

TEST_CASE("One edit after another is one step each", "[EditSettling]")
{
    EditSettling settling;
    std::ignore = settling.settled("one", false);

    REQUIRE(settling.settled("two", false) == std::optional<std::string>("one"));
    REQUIRE(settling.settled("three", false) == std::optional<std::string>("two"));
}
