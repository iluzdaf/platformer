#include "input/key_tracker.hpp"
#include "input/keys_down.hpp"

void KeyTracker::registerKey(int key)
{
    keyStates[key] = {};
}

void KeyTracker::poll(const KeysDown &keysDown)
{
    for (auto &[key, state] : keyStates)
    {
        bool isCurrentlyDown = keysDown(key);
        bool wasDown = state.down;
        state.justPressed = !wasDown && isCurrentlyDown;
        state.down = isCurrentlyDown;
    }
}

bool KeyTracker::isPressed(int key) const
{
    auto it = keyStates.find(key);
    return it != keyStates.end() && it->second.justPressed;
}

bool KeyTracker::isDown(int key) const
{
    auto it = keyStates.find(key);
    return it != keyStates.end() && it->second.down;
}
