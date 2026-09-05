#include <catch2/catch_test_macros.hpp>
#include <GLFW/glfw3.h>
#include <initializer_list>
#include <set>
#include <glm/gtc/matrix_transform.hpp>
#include "input/input_intentions.hpp"
#include "input/keyboard_intentions.hpp"
#include "input/keys_down.hpp"
#include "input/keys_unless_captured.hpp"

namespace
{
    KeysDown holding(std::initializer_list<int> keys)
    {
        std::set<int> down(keys);
        return [down](int key) { return down.contains(key); };
    }
}

TEST_CASE("Keys reach the game while nothing captures them", "[KeysUnlessCaptured]")
{
    KeysDown keys = keysUnlessCaptured(holding({GLFW_KEY_RIGHT}), false);

    REQUIRE(keys(GLFW_KEY_RIGHT));
    REQUIRE_FALSE(keys(GLFW_KEY_LEFT));
}

TEST_CASE("No key is down while the ui has the keyboard", "[KeysUnlessCaptured]")
{
    KeysDown keys = keysUnlessCaptured(holding({GLFW_KEY_RIGHT, GLFW_KEY_P}), true);

    REQUIRE_FALSE(keys(GLFW_KEY_RIGHT));
    REQUIRE_FALSE(keys(GLFW_KEY_P));
}

TEST_CASE("The player stops when the ui takes the keyboard mid-hold", "[KeysUnlessCaptured]")
{
    KeyboardIntentions keyboard;

    keyboard.process(keysUnlessCaptured(holding({GLFW_KEY_RIGHT}), false));
    REQUIRE(keyboard.getIntentions().direction == glm::vec2(1.0f, 0.0f));

    keyboard.process(keysUnlessCaptured(holding({GLFW_KEY_RIGHT}), true));
    REQUIRE(keyboard.getIntentions().direction == glm::vec2(0.0f));
}
