#pragma once

#include <vector>
#include "input/input_intentions.hpp"

struct InputStep
{
    float duration = 0.0f;
    InputIntentions pressed;
};

using InputProgram = std::vector<InputStep>;

InputProgram aJumpHeldFor(float seconds);

float durationOf(const InputProgram &program);

InputProgram cutShortAt(const InputProgram &program, float elapsed);

InputIntentions replaying(const InputProgram &program, float elapsed, float feetX, float towardsX);
