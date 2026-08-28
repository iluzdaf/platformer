#include <catch2/catch_test_macros.hpp>
#include <algorithm>
#include <random>
#include <cmath>
#include <vector>
#include <glm/glm.hpp>
#include "cameras/camera_shake.hpp"

namespace
{
    constexpr std::mt19937::result_type Seed = 20260828;

    std::vector<glm::vec2> shakeFor(CameraShake &shake, int steps, float deltaTime = 0.01f)
    {
        std::vector<glm::vec2> offsets;
        for (int step = 0; step < steps; ++step)
            offsets.push_back(shake.getOffset(deltaTime));

        return offsets;
    }
}

TEST_CASE("Still until it is told to shake", "[CameraShake]")
{
    CameraShake shake(Seed);

    REQUIRE_FALSE(shake.isActive());
    REQUIRE(shake.getOffset(0.01f) == glm::vec2(0.0f));
}

TEST_CASE("Shakes for as long as it was asked to", "[CameraShake]")
{
    CameraShake shake(Seed);
    shake.start(0.1f, 4.0f);

    REQUIRE(shake.isActive());

    shakeFor(shake, 9);
    REQUIRE(shake.isActive());

    shakeFor(shake, 2);
    REQUIRE_FALSE(shake.isActive());
}

TEST_CASE("Settles back to no offset at all", "[CameraShake]")
{
    CameraShake shake(Seed);
    shake.start(0.1f, 4.0f);

    shakeFor(shake, 20);

    REQUIRE(shake.getOffset(0.01f) == glm::vec2(0.0f));
}

TEST_CASE("Stays within the magnitude it was given", "[CameraShake]")
{
    CameraShake shake(Seed);
    shake.start(1.0f, 4.0f);

    float furthest = 0.0f;
    for (glm::vec2 offset : shakeFor(shake, 99))
        furthest = std::max({furthest, std::abs(offset.x), std::abs(offset.y)});

    REQUIRE(furthest <= 4.0f);
    REQUIRE(furthest > 3.0f);
}

TEST_CASE("Moves both ways on both axes", "[CameraShake]")
{
    CameraShake shake(Seed);
    shake.start(1.0f, 4.0f);

    bool left = false, right = false, up = false, down = false;
    for (glm::vec2 offset : shakeFor(shake, 99))
    {
        left = left || offset.x < -1.0f;
        right = right || offset.x > 1.0f;
        up = up || offset.y < -1.0f;
        down = down || offset.y > 1.0f;
    }

    REQUIRE(left);
    REQUIRE(right);
    REQUIRE(up);
    REQUIRE(down);
}

TEST_CASE("The same seed shakes the same way", "[CameraShake]")
{
    CameraShake one(Seed), other(Seed);
    one.start(1.0f, 4.0f);
    other.start(1.0f, 4.0f);

    REQUIRE(shakeFor(one, 50) == shakeFor(other, 50));
}

TEST_CASE("A different seed shakes differently", "[CameraShake]")
{
    CameraShake one(Seed), other(Seed + 1);
    one.start(1.0f, 4.0f);
    other.start(1.0f, 4.0f);

    REQUIRE(shakeFor(one, 50) != shakeFor(other, 50));
}

TEST_CASE("Two shakes made without a seed do not agree", "[CameraShake]")
{
    CameraShake one, other;
    one.start(1.0f, 4.0f);
    other.start(1.0f, 4.0f);

    REQUIRE(shakeFor(one, 50) != shakeFor(other, 50));
}

TEST_CASE("Starting again restarts the clock", "[CameraShake]")
{
    CameraShake shake(Seed);
    shake.start(0.1f, 4.0f);

    shakeFor(shake, 9);
    shake.start(0.1f, 4.0f);

    shakeFor(shake, 9);
    REQUIRE(shake.isActive());
}

TEST_CASE("Dies down rather than stopping dead", "[CameraShake]")
{
    CameraShake shake(Seed);
    shake.start(1.0f, 4.0f);

    auto furthestIn = [](const std::vector<glm::vec2> &offsets)
    {
        float furthest = 0.0f;
        for (glm::vec2 offset : offsets)
            furthest = std::max({furthest, std::abs(offset.x), std::abs(offset.y)});

        return furthest;
    };

    float atTheStart = furthestIn(shakeFor(shake, 33));
    furthestIn(shakeFor(shake, 33));
    float atTheEnd = furthestIn(shakeFor(shake, 33));

    REQUIRE(atTheStart > 3.0f);
    REQUIRE(atTheEnd < 1.5f);
}
