#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "physics/fixed_time_step.hpp"
using Catch::Approx;

TEST_CASE("FixedTimeStep divides time into correct steps", "[FixedTimeStep]")
{
    FixedTimeStep timestep(0.01f);
    float total = 0.0f;
    int steps = 0;

    timestep.run(0.035f, [&](float dt)
                 {
        total += dt;
        steps++; });
    REQUIRE(steps == 4);
    REQUIRE(total == Approx(0.035f));
}

TEST_CASE("FixedTimestep uses full delta if under max step", "[FixedTimeStep]")
{
    FixedTimeStep timestep(0.01f);
    float total = 0.0f;
    int steps = 0;

    timestep.run(0.008f, [&](float dt)
                 {
        total += dt;
        steps++; });
    REQUIRE(steps == 1);
    REQUIRE(total == Approx(0.008f));
}

TEST_CASE("FixedTimeStep handles exact multiples", "[FixedTimeStep]")
{
    FixedTimeStep timestep(0.01f);
    int steps = 0;

    timestep.run(0.03f, [&](float)
                 { steps++; });
    REQUIRE(steps == 3);
}