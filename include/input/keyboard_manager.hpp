#pragma once
#include <GLFW/glfw3.h>
#include <unordered_map>
#include <functional>

class KeyboardManager
{
public:
    using InputPoller = std::function<int(int)>;

    void registerKey(int key);
    void update(GLFWwindow *window);
    void update(const InputPoller& poller);
    bool isPressed(int key) const;
    bool isReleased(int key) const;
    bool isDown(int key) const;

private:
    struct KeyState
    {
        bool down = false;
        bool justPressed = false;
        bool justReleased = false;
    };

    std::unordered_map<int, KeyState> keyStates;
};