#pragma once
#include <unordered_map>
#include <functional>

struct GLFWwindow;

class KeyboardManager
{
public:
    using InputPoller = std::function<int(int)>;
    void registerKey(int key);
    void poll(GLFWwindow *window);
    void poll(const InputPoller &poller);
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