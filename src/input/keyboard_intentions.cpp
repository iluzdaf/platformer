#include <GLFW/glfw3.h>
#include "input/keyboard_intentions.hpp"
#include "input/keys_down.hpp"

KeyboardIntentions::KeyboardIntentions()
{
    keys.registerKey(GLFW_KEY_UP);
    keys.registerKey(GLFW_KEY_DOWN);
    keys.registerKey(GLFW_KEY_LEFT);
    keys.registerKey(GLFW_KEY_RIGHT);
    keys.registerKey(GLFW_KEY_Z);
    keys.registerKey(GLFW_KEY_X);
    keys.registerKey(GLFW_KEY_C);
}

void KeyboardIntentions::process(const KeysDown &keysDown)
{
    keys.poll(keysDown);

    intentions.jumpRequested = keys.isPressed(GLFW_KEY_C);
    intentions.jumpHeld = keys.isDown(GLFW_KEY_C);
    intentions.dashRequested = keys.isPressed(GLFW_KEY_X);
    intentions.climbRequested = keys.isDown(GLFW_KEY_Z);
    intentions.direction = {
        keys.isDown(GLFW_KEY_LEFT) ? -1.0f : (keys.isDown(GLFW_KEY_RIGHT) ? 1.0f : 0.0f),
        keys.isDown(GLFW_KEY_UP) ? -1.0f : (keys.isDown(GLFW_KEY_DOWN) ? 1.0f : 0.0f)};
}

InputIntentions KeyboardIntentions::getIntentions() const
{
    return intentions;
}