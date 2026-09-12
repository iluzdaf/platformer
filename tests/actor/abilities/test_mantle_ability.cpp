#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "actor/abilities/mantle_ability.hpp"
#include "actor/abilities/mantle_ability_data.hpp"
#include "actor/decided.hpp"
#include "helpers/abilities.hpp"
#include "input/input_intentions.hpp"

using Catch::Approx;

namespace
{
    MantleAbilityData lasting(float duration)
    {
        MantleAbilityData data;
        data.mantleDuration = duration;
        return data;
    }

    bool ended(const Decided &decided)
    {
        return !decided.mantle.active;
    }
}

TEST_CASE(
    "Pressing up while hanging at a ledge starts a mantle, which pulls straight up first",
    "[MantleAbility]")
{
    MantleAbilityData data;
    MantleAbility mantle(data);
    Decided decided = hanging();

    tick(mantle, pressingUp(), atALedge(WallSide::Right), decided);

    REQUIRE(decided.mantle.active);
    REQUIRE(decided.mantle.velocity == glm::vec2(0.0f, -data.mantleSpeed));
}

TEST_CASE("Halfway through, a mantle carries over onto the ledge", "[MantleAbility]")
{
    MantleAbilityData data = lasting(0.095f);
    MantleAbility mantle(data);
    Decided decided = hanging();
    tick(mantle, pressingUp(), atALedge(WallSide::Right), decided);

    tick(mantle, pressingUp(), atALedge(WallSide::Right), decided, 3);
    REQUIRE(decided.mantle.velocity == glm::vec2(0.0f, -data.mantleSpeed));

    tick(mantle, pressingUp(), atALedge(WallSide::Right), decided);
    REQUIRE(decided.mantle.velocity == glm::vec2(data.mantleSpeed, 0.0f));
}

TEST_CASE("A ledge on the left carries it left", "[MantleAbility]")
{
    MantleAbilityData data = lasting(0.095f);
    MantleAbility mantle(data);
    Decided decided = hanging();

    tick(mantle, pressingUp(), atALedge(WallSide::Left), decided, 5);

    REQUIRE(decided.mantle.velocity == glm::vec2(-data.mantleSpeed, 0.0f));
}

TEST_CASE("A mantle lasts as long as it says, and then lets go", "[MantleAbility]")
{
    MantleAbility mantle(lasting(0.095f));
    Decided decided = hanging();
    tick(mantle, pressingUp(), atALedge(WallSide::Right), decided);

    int ticksUntilItEnds =
        ticksUntil(mantle, pressingUp(), atALedge(WallSide::Right), decided, ended);

    REQUIRE(ticksUntilItEnds == 9);
    REQUIRE(decided.mantle.velocity == glm::vec2(0.0f));
}

TEST_CASE(
    "Once started, a mantle carries on though up is let go and the wall is gone",
    "[MantleAbility]")
{
    MantleAbilityData data;
    MantleAbility mantle(data);
    Decided decided = hanging();
    tick(mantle, pressingUp(), atALedge(WallSide::Right), decided);
    decided.wallHang.active = false;

    tick(mantle, InputIntentions{}, inTheAir(), decided);

    REQUIRE(decided.mantle.active);
    REQUIRE(decided.mantle.velocity == glm::vec2(0.0f, -data.mantleSpeed));
}

TEST_CASE("Halfway up a wall there is no ledge to pull onto", "[MantleAbility]")
{
    MantleAbility mantle(MantleAbilityData{});
    Decided decided = hanging();

    tick(mantle, pressingUp(), onAWall(WallSide::Right), decided);

    REQUIRE_FALSE(decided.mantle.active);
}

TEST_CASE("Not pressing up at a ledge leaves it hanging", "[MantleAbility]")
{
    MantleAbility mantle(MantleAbilityData{});
    Decided decided = hanging();

    tick(mantle, InputIntentions{}, atALedge(WallSide::Right), decided);
    REQUIRE_FALSE(decided.mantle.active);

    tick(mantle, pressingDown(), atALedge(WallSide::Right), decided);
    REQUIRE_FALSE(decided.mantle.active);
}

TEST_CASE("At a ledge without holding the wall there is no mantle", "[MantleAbility]")
{
    MantleAbility mantle(MantleAbilityData{});
    Decided decided;

    tick(mantle, pressingUp(), atALedge(WallSide::Right), decided);

    REQUIRE_FALSE(decided.mantle.active);
    REQUIRE(decided.mantle.velocity == glm::vec2(0.0f));
}

TEST_CASE("A mantle that goes nowhere or takes no time is refused", "[MantleAbility]")
{
    MantleAbilityData noSpeed;
    noSpeed.mantleSpeed = 0.0f;
    REQUIRE_THROWS_WITH(MantleAbility(noSpeed), Catch::Matchers::ContainsSubstring("mantleSpeed"));

    REQUIRE_THROWS_WITH(
        MantleAbility(lasting(0.0f)), Catch::Matchers::ContainsSubstring("mantleDuration"));
}
