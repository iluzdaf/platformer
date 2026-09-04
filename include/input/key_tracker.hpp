#pragma once
#include <unordered_map>
#include "input/keys_down.hpp"

class KeyTracker
{
public:
    void registerKey(int key);
    void poll(const KeysDown &keysDown);
    bool isPressed(int key) const;
    bool isDown(int key) const;

private:
    struct KeyState
    {
        bool down = false;
        bool justPressed = false;
    };
    std::unordered_map<int, KeyState> keyStates;
};
