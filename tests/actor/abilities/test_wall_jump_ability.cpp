#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "actor/abilities/wall_jump_ability.hpp"
#include "actor/abilities/wall_jump_ability_data.hpp"
#include "actor/ability_states.hpp"
#include "actor/observed.hpp"
#include "helpers/abilities.hpp"
#include "input/input_intentions.hpp"

using Catch::Approx;

namespace
{
    WallJumpAbilityData timedAs(float duration, float buffer, float coyote)
    {
        WallJumpAbilityData data;
        data.wallJumpDuration = duration;
        data.wallJumpBufferDuration = buffer;
        data.wallJumpCoyoteDuration = coyote;
        return data;
    }

    bool ended(const AbilityStates &states)
    {
        return !states.wallJump.active;
    }
}

TEST_CASE(
    "A wall jump pushes up and away from a wall it grips, when jump is held pressing away",
    "[WallJumpAbility]")
{
    WallJumpAbilityData data;

    WallJumpAbility offTheLeft(data);
    AbilityStates fromTheLeft;
    tick(offTheLeft, holdingJump(1.0f), onAWall(WallSide::Left), fromTheLeft);
    REQUIRE(fromTheLeft.wallJump.active);
    REQUIRE(fromTheLeft.wallJump.velocity.x == Approx(data.wallJumpHorizontalSpeed));
    REQUIRE(fromTheLeft.wallJump.velocity.y == Approx(data.wallJumpSpeed));

    WallJumpAbility offTheRight(data);
    AbilityStates fromTheRight;
    tick(offTheRight, holdingJump(-1.0f), onAWall(WallSide::Right), fromTheRight);
    REQUIRE(fromTheRight.wallJump.active);
    REQUIRE(fromTheRight.wallJump.velocity.x == Approx(-data.wallJumpHorizontalSpeed));
    REQUIRE(fromTheRight.wallJump.velocity.y == Approx(data.wallJumpSpeed));
}

TEST_CASE("A wall jump says so once", "[WallJumpAbility]")
{
    WallJumpAbility wallJump(WallJumpAbilityData{});
    AbilityStates states;

    tick(wallJump, holdingJump(1.0f), onAWall(WallSide::Left), states);
    REQUIRE(states.wallJump.emit);

    tick(wallJump, holdingJump(1.0f), inTheAir(), states);
    REQUIRE(states.wallJump.active);
    REQUIRE_FALSE(states.wallJump.emit);
}

TEST_CASE("Pressing toward the wall, or nowhere, is not a wall jump", "[WallJumpAbility]")
{
    WallJumpAbility wallJump(WallJumpAbilityData{});
    AbilityStates states;

    tick(wallJump, holdingJump(0.0f), onAWall(WallSide::Left), states);
    REQUIRE_FALSE(states.wallJump.active);

    tick(wallJump, holdingJump(-1.0f), onAWall(WallSide::Left), states);
    REQUIRE_FALSE(states.wallJump.active);
    REQUIRE(states.wallJump.velocity == glm::vec2(0.0f));
}

TEST_CASE("A wall it cannot grip is not jumped from, whichever way it presses", "[WallJumpAbility]")
{
    WallJumpAbility wallJump(WallJumpAbilityData{});
    AbilityStates states;
    Observed rememberingThatWall = onASlipperyWall(WallSide::Left);
    rememberingThatWall.contacts.wasLastWallLeft = true;

    tick(wallJump, holdingJump(1.0f), onASlipperyWall(WallSide::Left), states);
    REQUIRE_FALSE(states.wallJump.active);

    tick(wallJump, holdingJump(1.0f), rememberingThatWall, states);
    REQUIRE_FALSE(states.wallJump.active);
}

TEST_CASE("A wall jump is not made from the ground", "[WallJumpAbility]")
{
    WallJumpAbility wallJump(WallJumpAbilityData{});
    AbilityStates states;
    Observed standingAgainstIt = onAWall(WallSide::Left);
    standingAgainstIt.contacts.onGround = true;

    tick(wallJump, holdingJump(1.0f), standingAgainstIt, states);

    REQUIRE_FALSE(states.wallJump.active);
}

TEST_CASE("A wall jump held just before reaching a wall goes on reaching it", "[WallJumpAbility]")
{
    WallJumpAbility wallJump(timedAs(0.2f, 0.045f, 0.045f));
    AbilityStates states;
    tick(wallJump, holdingJump(1.0f), inTheAir(), states);
    tick(wallJump, InputIntentions{}, inTheAir(), states, 3);

    tick(wallJump, InputIntentions{}, onAWall(WallSide::Left), states);

    REQUIRE(states.wallJump.active);
    REQUIRE(states.wallJump.direction == 1.0f);
}

TEST_CASE("A wall jump held too long before reaching a wall is forgotten", "[WallJumpAbility]")
{
    WallJumpAbility wallJump(timedAs(0.2f, 0.045f, 0.045f));
    AbilityStates states;
    tick(wallJump, holdingJump(1.0f), inTheAir(), states);
    tick(wallJump, InputIntentions{}, inTheAir(), states, 4);

    tick(wallJump, InputIntentions{}, onAWall(WallSide::Left), states);

    REQUIRE_FALSE(states.wallJump.active);
}

TEST_CASE("A wall jump can still be made just after leaving a wall", "[WallJumpAbility]")
{
    WallJumpAbility wallJump(timedAs(0.2f, 0.045f, 0.045f));
    AbilityStates states;
    tick(wallJump, InputIntentions{}, onAWall(WallSide::Left), states);
    tick(wallJump, InputIntentions{}, justOffAWall(WallSide::Left), states, 3);

    tick(wallJump, holdingJump(1.0f), justOffAWall(WallSide::Left), states);

    REQUIRE(states.wallJump.active);
    REQUIRE(states.wallJump.direction == 1.0f);
}

TEST_CASE("Too long after leaving a wall, a wall jump is not made", "[WallJumpAbility]")
{
    WallJumpAbility wallJump(timedAs(0.2f, 0.045f, 0.045f));
    AbilityStates states;
    tick(wallJump, InputIntentions{}, onAWall(WallSide::Left), states);
    tick(wallJump, InputIntentions{}, justOffAWall(WallSide::Left), states, 4);

    tick(wallJump, holdingJump(1.0f), justOffAWall(WallSide::Left), states);

    REQUIRE_FALSE(states.wallJump.active);
}

TEST_CASE("Leaving a wall it cannot grip gives no time to jump from it", "[WallJumpAbility]")
{
    WallJumpAbility wallJump(WallJumpAbilityData{});
    AbilityStates states;
    tick(wallJump, InputIntentions{}, onASlipperyWall(WallSide::Left), states);

    tick(wallJump, holdingJump(1.0f), justOffAWall(WallSide::Left), states);

    REQUIRE_FALSE(states.wallJump.active);
}

TEST_CASE(
    "A wall jump just after leaving a wall pushes away from it, not from one it cannot grip",
    "[WallJumpAbility]")
{
    WallJumpAbility wallJump(WallJumpAbilityData{});
    AbilityStates states;
    tick(wallJump, InputIntentions{}, onAWall(WallSide::Left), states);
    Observed nowAgainstASlipperyOne = justOffAWall(WallSide::Left);
    nowAgainstASlipperyOne.contacts.touchingRightWall = true;

    tick(wallJump, holdingJump(1.0f), nowAgainstASlipperyOne, states);

    REQUIRE(states.wallJump.emit);
    REQUIRE(states.wallJump.direction == 1.0f);
}

TEST_CASE("A wall jump uses up the leeway for leaving the wall", "[WallJumpAbility]")
{
    WallJumpAbility wallJump(timedAs(0.045f, 0.1f, 0.1f));
    AbilityStates states;
    tick(wallJump, holdingJump(1.0f), onAWall(WallSide::Left), states);
    ticksUntil(wallJump, holdingJump(1.0f), justOffAWall(WallSide::Left), states, ended);

    tick(wallJump, holdingJump(1.0f), justOffAWall(WallSide::Left), states);

    REQUIRE_FALSE(states.wallJump.active);
}

TEST_CASE("A wall jump lasts as long as it says, and then ends", "[WallJumpAbility]")
{
    WallJumpAbility wallJump(timedAs(0.045f, 0.1f, 0.1f));
    AbilityStates states;
    tick(wallJump, holdingJump(1.0f), onAWall(WallSide::Left), states);

    int ticksUntilItEnds = ticksUntil(wallJump, holdingJump(1.0f), inTheAir(), states, ended);

    REQUIRE(ticksUntilItEnds == 4);
    REQUIRE(states.wallJump.velocity == glm::vec2(0.0f));
}

TEST_CASE(
    "A wall jump ends when it meets a wall on the side it is heading for",
    "[WallJumpAbility]")
{
    WallJumpAbility wallJump(WallJumpAbilityData{});
    AbilityStates states;
    tick(wallJump, holdingJump(1.0f), onAWall(WallSide::Left), states);

    tick(wallJump, holdingJump(-1.0f), onASlipperyWall(WallSide::Right), states);

    REQUIRE_FALSE(states.wallJump.active);
    REQUIRE(states.wallJump.velocity == glm::vec2(0.0f));
}

TEST_CASE("A wall jump that does not go up and away is refused", "[WallJumpAbility]")
{
    WallJumpAbilityData downwards;
    downwards.wallJumpSpeed = 0.0f;
    REQUIRE_THROWS_WITH(
        WallJumpAbility(downwards),
        Catch::Matchers::ContainsSubstring("A wall jump needs a speed upward"));

    WallJumpAbilityData intoTheWall;
    intoTheWall.wallJumpHorizontalSpeed = 0.0f;
    REQUIRE_THROWS_WITH(
        WallJumpAbility(intoTheWall),
        Catch::Matchers::ContainsSubstring("A wall jump needs a speed away from the wall"));
}

TEST_CASE("A wall jump whose press or wall is forgiven for no time is refused", "[WallJumpAbility]")
{
    REQUIRE_THROWS_WITH(
        WallJumpAbility(timedAs(0.2f, 0.0f, 0.1f)),
        Catch::Matchers::ContainsSubstring("A wall jump's buffer needs a length above 0"));
    REQUIRE_THROWS_WITH(
        WallJumpAbility(timedAs(0.2f, 0.1f, 0.0f)),
        Catch::Matchers::ContainsSubstring(
            "A wall jump's leeway off a wall needs a length above 0"));
}
