#include <algorithm>
#include "navigation/input_program.hpp"
#include "input/input_intentions.hpp"

InputProgram aJumpHeldFor(float seconds)
{
    InputIntentions jumping;
    jumping.jumpRequested = true;
    jumping.jumpHeld = true;
    return {{seconds, jumping}};
}

float durationOf(const InputProgram &program)
{
    float total = 0.0f;
    for (const InputStep &step : program)
        total += step.duration;
    return total;
}

InputProgram cutShortAt(const InputProgram &program, float elapsed)
{
    InputProgram cut;
    float startedAt = 0.0f;
    for (const InputStep &step : program)
    {
        if (startedAt >= elapsed)
            break;

        InputStep kept = step;
        kept.duration = std::min(step.duration, elapsed - startedAt);
        cut.push_back(kept);
        startedAt += step.duration;
    }
    return cut;
}

InputIntentions replaying(const InputProgram &program, float elapsed, float feetX, float towardsX)
{
    InputIntentions inputIntentions;
    float endsAt = 0.0f;
    for (const InputStep &step : program)
    {
        endsAt += step.duration;
        if (elapsed < endsAt)
        {
            inputIntentions = step.pressed;
            break;
        }
    }

    if (inputIntentions.direction.x == 0.0f && towardsX != feetX)
        inputIntentions.direction.x = towardsX > feetX ? 1.0f : -1.0f;

    return inputIntentions;
}
