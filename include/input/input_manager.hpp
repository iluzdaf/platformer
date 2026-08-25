#pragma once

#include "input/keyboard_manager.hpp"
#include "input/input_intentions.hpp"
#include "input/intention_source.hpp"

class InputManager : public IntentionSource
{
public:
    InputManager();
    void process(GLFWwindow *window);
    InputIntentions getIntentions() const override;

private:
    KeyboardManager keyboardManager;
    InputIntentions intentions;
};