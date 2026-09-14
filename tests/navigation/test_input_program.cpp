#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "input/input_intentions.hpp"
#include "navigation/input_program.hpp"

namespace
{
    InputIntentions pressingLeft()
    {
        InputIntentions inputIntentions;
        inputIntentions.direction.x = -1.0f;
        return inputIntentions;
    }
}

TEST_CASE("A jump held for a time presses jump for that long, then nothing", "[InputProgram]")
{
    InputProgram jump = aJumpHeldFor(0.05f);

    REQUIRE(replaying(jump, 0.0f, 0.0f, 0.0f).jumpHeld);
    REQUIRE(replaying(jump, 0.049f, 0.0f, 0.0f).jumpRequested);
    REQUIRE_FALSE(replaying(jump, 0.05f, 0.0f, 0.0f).jumpHeld);
    REQUIRE_FALSE(replaying(jump, 1.0f, 0.0f, 0.0f).jumpRequested);
}

TEST_CASE("A step that sets no direction is steered towards where the leg goes", "[InputProgram]")
{
    InputProgram jump = aJumpHeldFor(0.05f);

    REQUIRE(replaying(jump, 0.0f, 10.0f, 20.0f).direction.x == 1.0f);
    REQUIRE(replaying(jump, 0.0f, 10.0f, 0.0f).direction.x == -1.0f);
    REQUIRE(replaying(jump, 0.0f, 10.0f, 10.0f).direction.x == 0.0f);
    REQUIRE(replaying(jump, 1.0f, 10.0f, 20.0f).direction.x == 1.0f);
}

TEST_CASE("A step that sets a direction is sent as it is", "[InputProgram]")
{
    InputProgram backingOff{{0.05f, pressingLeft()}};

    REQUIRE(replaying(backingOff, 0.0f, 10.0f, 20.0f).direction.x == -1.0f);
    REQUIRE(replaying(backingOff, 0.05f, 10.0f, 20.0f).direction.x == 1.0f);
}

TEST_CASE("Each step is pressed in turn", "[InputProgram]")
{
    InputProgram program = aJumpHeldFor(0.05f);
    program.push_back({0.05f, pressingLeft()});

    REQUIRE(replaying(program, 0.02f, 10.0f, 20.0f).jumpHeld);
    REQUIRE(replaying(program, 0.02f, 10.0f, 20.0f).direction.x == 1.0f);
    REQUIRE_FALSE(replaying(program, 0.07f, 10.0f, 20.0f).jumpHeld);
    REQUIRE(replaying(program, 0.07f, 10.0f, 20.0f).direction.x == -1.0f);
    REQUIRE(durationOf(program) == Catch::Approx(0.1f));
}

TEST_CASE("A program cut short keeps only what came before", "[InputProgram]")
{
    InputProgram program = aJumpHeldFor(0.2f);
    program.push_back({0.3f, pressingLeft()});

    InputProgram intoTheSecond = cutShortAt(program, 0.25f);
    InputProgram withinTheFirst = cutShortAt(program, 0.1f);

    REQUIRE(intoTheSecond.size() == 2);
    REQUIRE(intoTheSecond.back().duration == Catch::Approx(0.05f));
    REQUIRE(intoTheSecond.back().pressed.direction.x == -1.0f);
    REQUIRE(withinTheFirst.size() == 1);
    REQUIRE(withinTheFirst.front().duration == Catch::Approx(0.1f));
    REQUIRE(cutShortAt(program, 1.0f).size() == 2);
    REQUIRE(durationOf(cutShortAt(program, 1.0f)) == Catch::Approx(0.5f));
}

TEST_CASE("Steering holds off within half a stride of where the leg goes", "[InputProgram]")
{
    InputProgram nothing;

    REQUIRE(replaying(nothing, 0.0f, 10.0f, 11.0f, 2.5f).direction.x == 0.0f);
    REQUIRE(replaying(nothing, 0.0f, 10.0f, 9.0f, 2.5f).direction.x == 0.0f);
    REQUIRE(replaying(nothing, 0.0f, 10.0f, 11.5f, 2.5f).direction.x == 1.0f);
    REQUIRE(replaying(nothing, 0.0f, 10.0f, 8.5f, 2.5f).direction.x == -1.0f);
}
