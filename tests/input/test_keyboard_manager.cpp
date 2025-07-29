#include <catch2/catch_test_macros.hpp>
#include <GLFW/glfw3.h>
#include "input/keyboard_manager.hpp"

TEST_CASE("KeyboardManager tracks key press and release", "[keyboard]")
{
    KeyboardManager keyboardManager;
    keyboardManager.registerKey(GLFW_KEY_SPACE);

    SECTION("Key press is detected")
    {
        keyboardManager.poll([](int _) { return GLFW_RELEASE; });
        keyboardManager.poll([](int key) { return key == GLFW_KEY_SPACE ? GLFW_PRESS : GLFW_RELEASE; });
        REQUIRE(keyboardManager.isPressed(GLFW_KEY_SPACE));
    }

    SECTION("Key release is detected")
    {
        keyboardManager.poll([](int _) { return GLFW_RELEASE; });
        keyboardManager.poll([](int key) { return key == GLFW_KEY_SPACE ? GLFW_PRESS : GLFW_RELEASE; });
        keyboardManager.poll([](int _) { return GLFW_RELEASE; });
        REQUIRE(keyboardManager.isReleased(GLFW_KEY_SPACE));
    }

    SECTION("Key press state resets correctly")
    {
        keyboardManager.poll([](int _) { return GLFW_RELEASE; });
        keyboardManager.poll([](int key) { return key == GLFW_KEY_SPACE ? GLFW_PRESS : GLFW_RELEASE; });
        REQUIRE(keyboardManager.isPressed(GLFW_KEY_SPACE));
        keyboardManager.poll([](int key) { return key == GLFW_KEY_SPACE ? GLFW_PRESS : GLFW_RELEASE; });
        REQUIRE_FALSE(keyboardManager.isPressed(GLFW_KEY_SPACE));
    }

    SECTION("Key release state resets correctly")
    {
        keyboardManager.poll([](int _) { return GLFW_RELEASE; });
        keyboardManager.poll([](int key) { return key == GLFW_KEY_SPACE ? GLFW_PRESS : GLFW_RELEASE; });
        keyboardManager.poll([](int _) { return GLFW_RELEASE; });
        REQUIRE(keyboardManager.isReleased(GLFW_KEY_SPACE));
        keyboardManager.poll([](int _) { return GLFW_RELEASE; });
        REQUIRE_FALSE(keyboardManager.isReleased(GLFW_KEY_SPACE));
    }
}