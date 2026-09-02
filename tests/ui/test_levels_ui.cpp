#include <optional>
#include <string>
#include <catch2/catch_test_macros.hpp>
#include "ui/switching_level.hpp"

namespace
{
    const std::optional<std::string> Nothing;
    const std::optional<std::string> Another{"levels/level3.json"};
    const std::optional<std::string> Held{"levels/level5.json"};
}

TEST_CASE("Picking a level with nothing unsaved goes there", "[LevelsUi]")
{
    SwitchingLevel decided = switching(Another, false, Nothing, false, false);

    REQUIRE(decided.loadNow == Another);
    REQUIRE_FALSE(decided.waitingOn.has_value());
}

TEST_CASE("Picking a level with unsaved changes waits rather than going", "[LevelsUi]")
{
    SwitchingLevel decided = switching(Another, true, Nothing, false, false);

    REQUIRE_FALSE(decided.loadNow.has_value());
    REQUIRE(decided.waitingOn == Another);
}

TEST_CASE("Switching goes to the level that was waiting", "[LevelsUi]")
{
    SwitchingLevel decided = switching(Nothing, true, Held, true, false);

    REQUIRE(decided.loadNow == Held);
    REQUIRE_FALSE(decided.waitingOn.has_value());
}

TEST_CASE("Cancelling goes nowhere and stops waiting", "[LevelsUi]")
{
    SwitchingLevel decided = switching(Nothing, true, Held, false, true);

    REQUIRE_FALSE(decided.loadNow.has_value());
    REQUIRE_FALSE(decided.waitingOn.has_value());
}

TEST_CASE("Waiting carries on until something is pressed", "[LevelsUi]")
{
    SwitchingLevel decided = switching(Nothing, true, Held, false, false);

    REQUIRE_FALSE(decided.loadNow.has_value());
    REQUIRE(decided.waitingOn == Held);
}

TEST_CASE("Saving while it waits stops it waiting", "[LevelsUi]")
{
    SwitchingLevel decided = switching(Nothing, false, Held, false, false);

    REQUIRE_FALSE(decided.loadNow.has_value());
    REQUIRE_FALSE(decided.waitingOn.has_value());
}

TEST_CASE("Picking again while it waits waits on the new one", "[LevelsUi]")
{
    SwitchingLevel decided = switching(Another, true, Held, false, false);

    REQUIRE_FALSE(decided.loadNow.has_value());
    REQUIRE(decided.waitingOn == Another);
}

TEST_CASE("Saving then picking goes straight there", "[LevelsUi]")
{
    SwitchingLevel decided = switching(Another, false, Held, false, false);

    REQUIRE(decided.loadNow == Another);
    REQUIRE_FALSE(decided.waitingOn.has_value());
}
