#include "game/actor/behaviors/input_behavior.hpp"

void InputBehavior::setIntentions(const InputIntentions &newIntentions)
{
    intentions = newIntentions;
}

InputIntentions InputBehavior::decide(float, const ActorBehaviorContext &)
{
    return intentions;
}
