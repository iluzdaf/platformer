#include <catch2/catch_test_macros.hpp>
#include <glm/glm.hpp>
#include <catch2/catch_approx.hpp>
#include "animations/sprite_animation.hpp"
using Catch::Approx;

TEST_CASE("Default Constucted SpriteAnimation behaves correctly", "[FrameAnimation]")
{
    SpriteAnimation spriteAnimation;
    REQUIRE(spriteAnimation.getUVStart() == glm::vec2(0, 0));
    REQUIRE(spriteAnimation.getUVEnd() == glm::vec2(1, 1));

    spriteAnimation.update(0.5f);
    REQUIRE(spriteAnimation.getUVStart() == glm::vec2(0, 0));
    REQUIRE(spriteAnimation.getUVEnd() == glm::vec2(1, 1));
}

TEST_CASE("SpriteAnimation returns correct UVs", "[SpriteAnimation]")
{
    SpriteAnimation anim(SpriteAnimationData{FrameAnimationData{{0, 1, 2}, 0.2f}, 32, 32, 96});

    SECTION("Frame 0 UV")
    {
        glm::vec2 start = anim.getUVStart();
        glm::vec2 end = anim.getUVEnd();

        REQUIRE(start == glm::vec2(0.0f, 0.0f));
        REQUIRE(end == glm::vec2(1.0f / 3.0f, 1.0f / 3.0f));
    }

    SECTION("After update, UV moves to frame 1")
    {
        anim.update(0.25f);
        glm::vec2 start = anim.getUVStart();
        glm::vec2 expectedStart = glm::vec2(1.0f / 3.0f, 0.0f);

        REQUIRE(start.x == Approx(expectedStart.x));
        REQUIRE(start.y == Approx(expectedStart.y));
    }

    SECTION("UV stays within texture range")
    {
        anim.update(0.5f);
        glm::vec2 start = anim.getUVStart();
        REQUIRE(start.x <= 1.0f);
        REQUIRE(start.y <= 1.0f);
    }
}