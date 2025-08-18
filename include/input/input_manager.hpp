#pragma once

#include "input/keyboard_manager.hpp"
#include "input/input_intentions.hpp"

class InputManager
{
public:
    InputManager();
    void process(GLFWwindow *window);
    InputIntentions getIntentions() const;

private:
    KeyboardManager keyboardManager;
    InputIntentions intentions;
};