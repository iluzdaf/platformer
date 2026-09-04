#pragma once

#include "input/key_tracker.hpp"
#include "input/keys_down.hpp"
#include "input/input_intentions.hpp"
#include "input/intention_source.hpp"

class KeyboardIntentions : public IntentionSource
{
public:
    KeyboardIntentions();
    void process(const KeysDown &keysDown);
    InputIntentions getIntentions() const override;

private:
    KeyTracker keys;
    InputIntentions intentions;
};
