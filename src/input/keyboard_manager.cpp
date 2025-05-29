#include "input/keyboard_manager.hpp"

void KeyboardManager::registerKey(int key)
{
    keyStates[key] = {};
}

void KeyboardManager::update(GLFWwindow *window)
{
    update([window](int key)
           { return glfwGetKey(window, key); });
}

void KeyboardManager::update(const InputPoller &poller)
{
    for (auto &[key, state] : keyStates)
    {
        bool isCurrentlyDown = poller(key) == GLFW_PRESS;
        bool wasDown = state.down;
        state.justPressed = !wasDown && isCurrentlyDown;
        state.justReleased = wasDown && !isCurrentlyDown;
        state.down = isCurrentlyDown;
    }
}

bool KeyboardManager::isPressed(int key) const
{
    auto it = keyStates.find(key);
    return it != keyStates.end() && it->second.justPressed;
}

bool KeyboardManager::isReleased(int key) const
{
    auto it = keyStates.find(key);
    return it != keyStates.end() && it->second.justReleased;
}

bool KeyboardManager::isDown(int key) const
{
    auto it = keyStates.find(key);
    return it != keyStates.end() && it->second.down;
}