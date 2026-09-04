#include <catch2/catch_test_macros.hpp>
#include <GLFW/glfw3.h>
#include <initializer_list>
#include <set>
#include "input/input_intentions.hpp"
#include "input/keyboard_intentions.hpp"
#include "input/keyboard_manager.hpp"

namespace
{
    KeyboardManager::InputPoller holding(std::initializer_list<int> keys)
    {
        std::set<int> down(keys);
        return [down](int key) { return down.contains(key) ? GLFW_PRESS : GLFW_RELEASE; };
    }
}

TEST_CASE("Nothing pressed asks for nothing", "[KeyboardIntentions]")
{
    KeyboardIntentions keyboard;

    keyboard.process(holding({}));
    InputIntentions asked = keyboard.getIntentions();

    REQUIRE(asked.direction == glm::vec2(0.0f));
    REQUIRE_FALSE(asked.jumpRequested);
    REQUIRE_FALSE(asked.jumpHeld);
    REQUIRE_FALSE(asked.dashRequested);
    REQUIRE_FALSE(asked.climbRequested);
}

TEST_CASE("The arrows say which way", "[KeyboardIntentions]")
{
    KeyboardIntentions keyboard;

    keyboard.process(holding({GLFW_KEY_RIGHT}));
    REQUIRE(keyboard.getIntentions().direction == glm::vec2(1.0f, 0.0f));

    keyboard.process(holding({GLFW_KEY_LEFT}));
    REQUIRE(keyboard.getIntentions().direction == glm::vec2(-1.0f, 0.0f));

    keyboard.process(holding({GLFW_KEY_UP}));
    REQUIRE(keyboard.getIntentions().direction == glm::vec2(0.0f, -1.0f));

    keyboard.process(holding({GLFW_KEY_DOWN}));
    REQUIRE(keyboard.getIntentions().direction == glm::vec2(0.0f, 1.0f));
}

TEST_CASE("Left beats right and up beats down when both are held", "[KeyboardIntentions]")
{
    KeyboardIntentions keyboard;

    keyboard.process(holding({GLFW_KEY_LEFT, GLFW_KEY_RIGHT, GLFW_KEY_UP, GLFW_KEY_DOWN}));

    REQUIRE(keyboard.getIntentions().direction == glm::vec2(-1.0f, -1.0f));
}

TEST_CASE("C asks for a jump the frame it is pressed and holds it after", "[KeyboardIntentions]")
{
    KeyboardIntentions keyboard;

    keyboard.process(holding({GLFW_KEY_C}));
    REQUIRE(keyboard.getIntentions().jumpRequested);
    REQUIRE(keyboard.getIntentions().jumpHeld);

    keyboard.process(holding({GLFW_KEY_C}));
    REQUIRE_FALSE(keyboard.getIntentions().jumpRequested);
    REQUIRE(keyboard.getIntentions().jumpHeld);
}

TEST_CASE("X asks for a dash only the frame it is pressed", "[KeyboardIntentions]")
{
    KeyboardIntentions keyboard;

    keyboard.process(holding({GLFW_KEY_X}));
    REQUIRE(keyboard.getIntentions().dashRequested);

    keyboard.process(holding({GLFW_KEY_X}));
    REQUIRE_FALSE(keyboard.getIntentions().dashRequested);
}

TEST_CASE("Z asks to climb for as long as it is held", "[KeyboardIntentions]")
{
    KeyboardIntentions keyboard;

    keyboard.process(holding({GLFW_KEY_Z}));
    REQUIRE(keyboard.getIntentions().climbRequested);

    keyboard.process(holding({GLFW_KEY_Z}));
    REQUIRE(keyboard.getIntentions().climbRequested);

    keyboard.process(holding({}));
    REQUIRE_FALSE(keyboard.getIntentions().climbRequested);
}
