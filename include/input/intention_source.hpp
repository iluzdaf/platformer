#pragma once

#include "input/input_intentions.hpp"

class IntentionSource
{
public:
    virtual ~IntentionSource() = default;
    virtual InputIntentions getIntentions() const = 0;
};
