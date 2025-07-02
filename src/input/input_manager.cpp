#include <GLFW/glfw3.h>
#include "input/input_manager.hpp"

InputManager::InputManager()
{
    keyboardManager.registerKey(GLFW_KEY_UP);
    keyboardManager.registerKey(GLFW_KEY_DOWN);
    keyboardManager.registerKey(GLFW_KEY_LEFT);
    keyboardManager.registerKey(GLFW_KEY_RIGHT);
    keyboardManager.registerKey(GLFW_KEY_Z);
    keyboardManager.registerKey(GLFW_KEY_X);
    keyboardManager.registerKey(GLFW_KEY_C);
}

void InputManager::process(GLFWwindow *window)
{
    keyboardManager.poll(window);

    intentions.jumpRequested = keyboardManager.isPressed(GLFW_KEY_C);
    intentions.jumpHeld = keyboardManager.isDown(GLFW_KEY_C);
    intentions.dashRequested = keyboardManager.isPressed(GLFW_KEY_X);
    intentions.climbRequested = keyboardManager.isDown(GLFW_KEY_Z);
    intentions.direction = {
        keyboardManager.isDown(GLFW_KEY_LEFT) ? -1.0f : (keyboardManager.isDown(GLFW_KEY_RIGHT) ? 1.0f : 0.0f),
        keyboardManager.isDown(GLFW_KEY_UP) ? -1.0f : (keyboardManager.isDown(GLFW_KEY_DOWN) ? 1.0f : 0.0f)};
}

InputIntentions InputManager::getIntentions() const
{
    return intentions;
}