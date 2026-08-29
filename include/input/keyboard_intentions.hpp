#pragma once

#include "input/keyboard_manager.hpp"
#include "input/input_intentions.hpp"
#include "input/intention_source.hpp"

class KeyboardIntentions : public IntentionSource
{
public:
    KeyboardIntentions();
    void process(GLFWwindow *window);
    InputIntentions getIntentions() const override;

private:
    KeyboardManager keyboardManager;
    InputIntentions intentions;
};