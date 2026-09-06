#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <catch2/catch_approx.hpp>
#include <vector>
#include "timing/fixed_time_step.hpp"

using Catch::Approx;

namespace
{
    std::vector<float> stepsOf(FixedTimeStep &timestep, float deltaTime)
    {
        std::vector<float> steps;
        timestep.run(deltaTime, [&](float dt) { steps.push_back(dt); });
        return steps;
    }
}

TEST_CASE("Time is run in whole steps and the rest is carried", "[FixedTimeStep]")
{
    FixedTimeStep timestep(0.01f);

    REQUIRE(stepsOf(timestep, 0.035f).size() == 3);
    REQUIRE(stepsOf(timestep, 0.005f).size() == 1);
}

TEST_CASE("Every step is the same length", "[FixedTimeStep]")
{
    FixedTimeStep timestep(0.01f);

    for (float dt : stepsOf(timestep, 0.035f))
        REQUIRE(dt == Approx(0.01f));
}

TEST_CASE("Less than a step runs nothing until enough has built up", "[FixedTimeStep]")
{
    FixedTimeStep timestep(0.01f);

    REQUIRE(stepsOf(timestep, 0.008f).empty());
    REQUIRE(stepsOf(timestep, 0.008f).size() == 1);
}

TEST_CASE("An exact multiple runs exactly that many", "[FixedTimeStep]")
{
    FixedTimeStep timestep(0.01f);

    REQUIRE(stepsOf(timestep, 0.03f).size() == 3);
    REQUIRE(stepsOf(timestep, 0.03f).size() == 3);
}

TEST_CASE("Time carried over many frames is never lost", "[FixedTimeStep]")
{
    FixedTimeStep timestep(0.01f);
    std::size_t steps = 0;

    for (int frame = 0; frame < 60; ++frame)
        steps += stepsOf(timestep, 1.0f / 60.0f).size();

    REQUIRE(steps == 100);
}

TEST_CASE("The step the game runs is the one the graphs are built on", "[FixedTimeStep]")
{
    REQUIRE(FixedTimeStep().getMaxStep() == PhysicsStep);
}
