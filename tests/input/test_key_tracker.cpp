#include <catch2/catch_test_macros.hpp>
#include <GLFW/glfw3.h>
#include "input/key_tracker.hpp"

TEST_CASE("KeyTracker tracks key press and release", "[keyboard]")
{
    KeyTracker keys;
    keys.registerKey(GLFW_KEY_SPACE);

    SECTION("Key press is detected")
    {
        keys.poll([](int) { return false; });
        keys.poll([](int key) { return key == GLFW_KEY_SPACE; });
        REQUIRE(keys.isPressed(GLFW_KEY_SPACE));
        REQUIRE(keys.isDown(GLFW_KEY_SPACE));
    }

    SECTION("Key press state resets correctly")
    {
        keys.poll([](int) { return false; });
        keys.poll([](int key) { return key == GLFW_KEY_SPACE; });
        REQUIRE(keys.isPressed(GLFW_KEY_SPACE));
        keys.poll([](int key) { return key == GLFW_KEY_SPACE; });
        REQUIRE_FALSE(keys.isPressed(GLFW_KEY_SPACE));
        REQUIRE(keys.isDown(GLFW_KEY_SPACE));
    }

    SECTION("A key nobody registered is never down")
    {
        keys.poll([](int) { return true; });
        REQUIRE_FALSE(keys.isDown(GLFW_KEY_A));
        REQUIRE_FALSE(keys.isPressed(GLFW_KEY_A));
    }
}
