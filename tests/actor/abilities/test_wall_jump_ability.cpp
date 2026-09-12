#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "actor/abilities/wall_jump_ability.hpp"
#include "actor/abilities/wall_jump_ability_data.hpp"
#include "actor/decided.hpp"
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

    bool ended(const Decided &decided)
    {
        return !decided.wallJump.active;
    }
}

TEST_CASE(
    "A wall jump pushes up and away from a wall it grips, when jump is held pressing away",
    "[WallJumpAbility]")
{
    WallJumpAbilityData data;

    WallJumpAbility offTheLeft(data);
    Decided fromTheLeft;
    tick(offTheLeft, holdingJump(1.0f), onAWall(WallSide::Left), fromTheLeft);
    REQUIRE(fromTheLeft.wallJump.active);
    REQUIRE(fromTheLeft.wallJump.velocity.x == Approx(data.wallJumpHorizontalSpeed));
    REQUIRE(fromTheLeft.wallJump.velocity.y == Approx(data.wallJumpSpeed));

    WallJumpAbility offTheRight(data);
    Decided fromTheRight;
    tick(offTheRight, holdingJump(-1.0f), onAWall(WallSide::Right), fromTheRight);
    REQUIRE(fromTheRight.wallJump.active);
    REQUIRE(fromTheRight.wallJump.velocity.x == Approx(-data.wallJumpHorizontalSpeed));
    REQUIRE(fromTheRight.wallJump.velocity.y == Approx(data.wallJumpSpeed));
}

TEST_CASE("A wall jump says so once", "[WallJumpAbility]")
{
    WallJumpAbility wallJump(WallJumpAbilityData{});
    Decided decided;

    tick(wallJump, holdingJump(1.0f), onAWall(WallSide::Left), decided);
    REQUIRE(decided.wallJump.emit);

    tick(wallJump, holdingJump(1.0f), inTheAir(), decided);
    REQUIRE(decided.wallJump.active);
    REQUIRE_FALSE(decided.wallJump.emit);
}

TEST_CASE("Pressing toward the wall, or nowhere, is not a wall jump", "[WallJumpAbility]")
{
    WallJumpAbility wallJump(WallJumpAbilityData{});
    Decided decided;

    tick(wallJump, holdingJump(0.0f), onAWall(WallSide::Left), decided);
    REQUIRE_FALSE(decided.wallJump.active);

    tick(wallJump, holdingJump(-1.0f), onAWall(WallSide::Left), decided);
    REQUIRE_FALSE(decided.wallJump.active);
    REQUIRE(decided.wallJump.velocity == glm::vec2(0.0f));
}

TEST_CASE("A wall it cannot grip is not jumped from, whichever way it presses", "[WallJumpAbility]")
{
    WallJumpAbility wallJump(WallJumpAbilityData{});
    Decided decided;
    Observed rememberingThatWall = onASlipperyWall(WallSide::Left);
    rememberingThatWall.contacts.wasLastWallLeft = true;

    tick(wallJump, holdingJump(1.0f), onASlipperyWall(WallSide::Left), decided);
    REQUIRE_FALSE(decided.wallJump.active);

    tick(wallJump, holdingJump(1.0f), rememberingThatWall, decided);
    REQUIRE_FALSE(decided.wallJump.active);
}

TEST_CASE("A wall jump is not made from the ground", "[WallJumpAbility]")
{
    WallJumpAbility wallJump(WallJumpAbilityData{});
    Decided decided;
    Observed standingAgainstIt = onAWall(WallSide::Left);
    standingAgainstIt.contacts.onGround = true;

    tick(wallJump, holdingJump(1.0f), standingAgainstIt, decided);

    REQUIRE_FALSE(decided.wallJump.active);
}

TEST_CASE("A wall jump held just before reaching a wall goes on reaching it", "[WallJumpAbility]")
{
    WallJumpAbility wallJump(timedAs(0.2f, 0.045f, 0.045f));
    Decided decided;
    tick(wallJump, holdingJump(1.0f), inTheAir(), decided);
    tick(wallJump, InputIntentions{}, inTheAir(), decided, 3);

    tick(wallJump, InputIntentions{}, onAWall(WallSide::Left), decided);

    REQUIRE(decided.wallJump.active);
    REQUIRE(decided.wallJump.direction == 1.0f);
}

TEST_CASE("A wall jump held too long before reaching a wall is forgotten", "[WallJumpAbility]")
{
    WallJumpAbility wallJump(timedAs(0.2f, 0.045f, 0.045f));
    Decided decided;
    tick(wallJump, holdingJump(1.0f), inTheAir(), decided);
    tick(wallJump, InputIntentions{}, inTheAir(), decided, 4);

    tick(wallJump, InputIntentions{}, onAWall(WallSide::Left), decided);

    REQUIRE_FALSE(decided.wallJump.active);
}

TEST_CASE("A wall jump can still be made just after leaving a wall", "[WallJumpAbility]")
{
    WallJumpAbility wallJump(timedAs(0.2f, 0.045f, 0.045f));
    Decided decided;
    tick(wallJump, InputIntentions{}, onAWall(WallSide::Left), decided);
    tick(wallJump, InputIntentions{}, justOffAWall(WallSide::Left), decided, 3);

    tick(wallJump, holdingJump(1.0f), justOffAWall(WallSide::Left), decided);

    REQUIRE(decided.wallJump.active);
    REQUIRE(decided.wallJump.direction == 1.0f);
}

TEST_CASE("Too long after leaving a wall, a wall jump is not made", "[WallJumpAbility]")
{
    WallJumpAbility wallJump(timedAs(0.2f, 0.045f, 0.045f));
    Decided decided;
    tick(wallJump, InputIntentions{}, onAWall(WallSide::Left), decided);
    tick(wallJump, InputIntentions{}, justOffAWall(WallSide::Left), decided, 4);

    tick(wallJump, holdingJump(1.0f), justOffAWall(WallSide::Left), decided);

    REQUIRE_FALSE(decided.wallJump.active);
}

TEST_CASE("Leaving a wall it cannot grip gives no time to jump from it", "[WallJumpAbility]")
{
    WallJumpAbility wallJump(WallJumpAbilityData{});
    Decided decided;
    tick(wallJump, InputIntentions{}, onASlipperyWall(WallSide::Left), decided);

    tick(wallJump, holdingJump(1.0f), justOffAWall(WallSide::Left), decided);

    REQUIRE_FALSE(decided.wallJump.active);
}

TEST_CASE(
    "A wall jump just after leaving a wall pushes away from it, not from one it cannot grip",
    "[WallJumpAbility]")
{
    WallJumpAbility wallJump(WallJumpAbilityData{});
    Decided decided;
    tick(wallJump, InputIntentions{}, onAWall(WallSide::Left), decided);
    Observed nowAgainstASlipperyOne = justOffAWall(WallSide::Left);
    nowAgainstASlipperyOne.contacts.touchingRightWall = true;

    tick(wallJump, holdingJump(1.0f), nowAgainstASlipperyOne, decided);

    REQUIRE(decided.wallJump.emit);
    REQUIRE(decided.wallJump.direction == 1.0f);
}

TEST_CASE("A wall jump uses up the leeway for leaving the wall", "[WallJumpAbility]")
{
    WallJumpAbility wallJump(timedAs(0.045f, 0.1f, 0.1f));
    Decided decided;
    tick(wallJump, holdingJump(1.0f), onAWall(WallSide::Left), decided);
    ticksUntil(wallJump, holdingJump(1.0f), justOffAWall(WallSide::Left), decided, ended);

    tick(wallJump, holdingJump(1.0f), justOffAWall(WallSide::Left), decided);

    REQUIRE_FALSE(decided.wallJump.active);
}

TEST_CASE("A wall jump lasts as long as it says, and then ends", "[WallJumpAbility]")
{
    WallJumpAbility wallJump(timedAs(0.045f, 0.1f, 0.1f));
    Decided decided;
    tick(wallJump, holdingJump(1.0f), onAWall(WallSide::Left), decided);

    int ticksUntilItEnds = ticksUntil(wallJump, holdingJump(1.0f), inTheAir(), decided, ended);

    REQUIRE(ticksUntilItEnds == 4);
    REQUIRE(decided.wallJump.velocity == glm::vec2(0.0f));
}

TEST_CASE(
    "A wall jump ends when it meets a wall on the side it is heading for",
    "[WallJumpAbility]")
{
    WallJumpAbility wallJump(WallJumpAbilityData{});
    Decided decided;
    tick(wallJump, holdingJump(1.0f), onAWall(WallSide::Left), decided);

    tick(wallJump, holdingJump(-1.0f), onASlipperyWall(WallSide::Right), decided);

    REQUIRE_FALSE(decided.wallJump.active);
    REQUIRE(decided.wallJump.velocity == glm::vec2(0.0f));
}

TEST_CASE("A wall jump that does not go up and away is refused", "[WallJumpAbility]")
{
    WallJumpAbilityData downwards;
    downwards.wallJumpSpeed = 0.0f;
    REQUIRE_THROWS_WITH(
        WallJumpAbility(downwards), Catch::Matchers::ContainsSubstring("wallJumpSpeed"));

    WallJumpAbilityData intoTheWall;
    intoTheWall.wallJumpHorizontalSpeed = 0.0f;
    REQUIRE_THROWS_WITH(
        WallJumpAbility(intoTheWall),
        Catch::Matchers::ContainsSubstring("wallJumpHorizontalSpeed"));
}

TEST_CASE("A wall jump whose press or wall is forgiven for no time is refused", "[WallJumpAbility]")
{
    REQUIRE_THROWS_WITH(
        WallJumpAbility(timedAs(0.2f, 0.0f, 0.1f)),
        Catch::Matchers::ContainsSubstring("grace period"));
    REQUIRE_THROWS_WITH(
        WallJumpAbility(timedAs(0.2f, 0.1f, 0.0f)),
        Catch::Matchers::ContainsSubstring("grace period"));
}
