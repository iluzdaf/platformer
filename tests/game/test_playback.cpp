#include <catch2/catch_test_macros.hpp>
#include <vector>
#include "game/playback.hpp"

namespace
{
    struct Ran
    {
        int began = 0, ended = 0;
        std::vector<float> steps;

        void advance(Playback &playback, float deltaTime)
        {
            playback.advance(
                deltaTime,
                [this] { ++began; },
                [this](float dt) { steps.push_back(dt); },
                [this](float) { ++ended; });
        }
    };
}

TEST_CASE("Playing runs the simulation", "[Playback]")
{
    Playback playback;
    Ran ran;

    ran.advance(playback, 0.05f);

    REQUIRE(ran.began == 1);
    REQUIRE(ran.ended == 1);
    REQUIRE(ran.steps.size() == 5);
}

TEST_CASE("Paused runs nothing at all", "[Playback]")
{
    Playback playback;
    Ran ran;
    playback.pause();

    ran.advance(playback, 0.05f);

    REQUIRE(ran.began == 0);
    REQUIRE(ran.ended == 0);
    REQUIRE(ran.steps.empty());
}

TEST_CASE("A step is one step and no more", "[Playback]")
{
    Playback playback;
    Ran ran;
    playback.step();

    ran.advance(playback, 0.05f);

    REQUIRE(ran.steps.size() == 1);
    REQUIRE(ran.began == 1);
    REQUIRE(ran.ended == 1);
}

TEST_CASE("Stepping leaves it stopped where it got to", "[Playback]")
{
    Playback playback;
    Ran ran;
    playback.step();
    ran.advance(playback, 0.05f);

    ran.advance(playback, 0.05f);

    REQUIRE(playback.isPaused());
    REQUIRE(ran.steps.size() == 1);
}

TEST_CASE("A step from a standing start does not skip ahead", "[Playback]")
{
    Playback playback;
    Ran ran;
    playback.step();

    ran.advance(playback, 5.0f);

    REQUIRE(ran.steps.size() == 1);
    REQUIRE(ran.steps.front() < 0.02f);
}

TEST_CASE("Playing again picks up where it stopped", "[Playback]")
{
    Playback playback;
    Ran ran;
    playback.pause();
    ran.advance(playback, 0.05f);

    playback.play();
    ran.advance(playback, 0.05f);

    REQUIRE_FALSE(playback.isPaused());
    REQUIRE(ran.steps.size() == 5);
}

TEST_CASE("It starts playing", "[Playback]")
{
    Playback playback;

    REQUIRE_FALSE(playback.isPaused());
}
