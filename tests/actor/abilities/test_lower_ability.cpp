#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "actor/abilities/lower_ability.hpp"
#include "actor/abilities/lower_ability_data.hpp"
#include "actor/abilities/ability_states.hpp"
#include "actor/observed.hpp"
#include "helpers/abilities.hpp"
#include "input/input_intentions.hpp"

namespace
{
    LowerAbilityData lasting(float duration)
    {
        LowerAbilityData data;
        data.lowerDuration = duration;
        return data;
    }

    bool ended(const AbilityStates &states)
    {
        return !states.lower.active;
    }

    Observed onTheGroundBy(WallSide side)
    {
        Observed observed = onAWall(side);
        observed.contacts.onGround = true;
        return observed;
    }
}

TEST_CASE(
    "Pressing down to climb at an edge starts a lower, which carries over the edge first",
    "[LowerAbility]")
{
    LowerAbilityData data;
    LowerAbility lower(data);
    AbilityStates states;

    tick(lower, pressingDownToClimb(), atAnEdge(WallSide::Left), states);

    REQUIRE(states.lower.active);
    REQUIRE(states.lower.velocity == glm::vec2(-data.lowerSpeed, 0.0f));
}

TEST_CASE("Once off the ground, a lower drops straight down", "[LowerAbility]")
{
    LowerAbilityData data;
    LowerAbility lower(data);
    AbilityStates states;
    tick(lower, pressingDownToClimb(), atAnEdge(WallSide::Left), states);

    tick(lower, InputIntentions{}, inTheAir(), states);

    REQUIRE(states.lower.active);
    REQUIRE(states.lower.velocity == glm::vec2(0.0f, data.lowerSpeed));
}

TEST_CASE("An edge on the right carries it right", "[LowerAbility]")
{
    LowerAbilityData data;
    LowerAbility lower(data);
    AbilityStates states;

    tick(lower, pressingDownToClimb(), atAnEdge(WallSide::Right), states);

    REQUIRE(states.lower.velocity == glm::vec2(data.lowerSpeed, 0.0f));
}

TEST_CASE("A lower ends once it grips the wall it went over", "[LowerAbility]")
{
    LowerAbility lower(LowerAbilityData{});
    AbilityStates states;
    tick(lower, pressingDownToClimb(), atAnEdge(WallSide::Left), states);
    tick(lower, pressingDownToClimb(), inTheAir(), states);

    tick(lower, pressingDownToClimb(), onAWall(WallSide::Left), states);
    REQUIRE(states.lower.active);

    tick(lower, pressingDownToClimb(), onAWall(WallSide::Right), states);
    REQUIRE_FALSE(states.lower.active);
    REQUIRE(states.lower.velocity == glm::vec2(0.0f));
}

TEST_CASE("A lower still inside the corner it went over carries on down", "[LowerAbility]")
{
    LowerAbility lower(LowerAbilityData{});
    AbilityStates states;
    tick(lower, pressingDownToClimb(), atAnEdge(WallSide::Left), states);
    Observed insideTheCorner = onAWall(WallSide::Right);
    insideTheCorner.contacts.touchingLeftWall = true;

    tick(lower, pressingDownToClimb(), insideTheCorner, states);

    REQUIRE(states.lower.active);
}

TEST_CASE("A lower still on the ground does not stop for a wall", "[LowerAbility]")
{
    LowerAbility lower(LowerAbilityData{});
    AbilityStates states;
    tick(lower, pressingDownToClimb(), atAnEdge(WallSide::Left), states);

    tick(lower, pressingDownToClimb(), onTheGroundBy(WallSide::Right), states);

    REQUIRE(states.lower.active);
}

TEST_CASE("A lower lasts no longer than it says, and then lets go", "[LowerAbility]")
{
    LowerAbility lower(lasting(0.095f));
    AbilityStates states;
    tick(lower, pressingDownToClimb(), atAnEdge(WallSide::Left), states);

    int ticksUntilItEnds = ticksUntil(lower, pressingDownToClimb(), inTheAir(), states, ended);

    REQUIRE(ticksUntilItEnds == 9);
    REQUIRE(states.lower.velocity == glm::vec2(0.0f));
}

TEST_CASE("There is no lower without pressing down and asking to climb", "[LowerAbility]")
{
    LowerAbility lower(LowerAbilityData{});
    AbilityStates states;

    tick(lower, pressingDown(), atAnEdge(WallSide::Left), states);
    REQUIRE_FALSE(states.lower.active);

    InputIntentions climbingUp = pressingUp();
    climbingUp.climbRequested = true;
    tick(lower, climbingUp, atAnEdge(WallSide::Left), states);
    REQUIRE_FALSE(states.lower.active);
}

TEST_CASE("There is no lower away from an edge or off the ground", "[LowerAbility]")
{
    LowerAbility lower(LowerAbilityData{});
    AbilityStates states;

    tick(lower, pressingDownToClimb(), onTheGround(), states);
    REQUIRE_FALSE(states.lower.active);

    Observed inTheAirAtAnEdge = atAnEdge(WallSide::Left);
    inTheAirAtAnEdge.contacts.onGround = false;
    tick(lower, pressingDownToClimb(), inTheAirAtAnEdge, states);
    REQUIRE_FALSE(states.lower.active);
}

TEST_CASE("A lower that goes nowhere or takes no time is refused", "[LowerAbility]")
{
    LowerAbilityData noSpeed;
    noSpeed.lowerSpeed = 0.0f;
    REQUIRE_THROWS_WITH(
        LowerAbility(noSpeed), Catch::Matchers::ContainsSubstring("A lower needs a speed above 0"));

    REQUIRE_THROWS_WITH(
        LowerAbility(lasting(0.0f)),
        Catch::Matchers::ContainsSubstring("A lower needs a duration above 0"));
}
