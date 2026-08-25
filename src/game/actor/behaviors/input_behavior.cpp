#include "game/actor/behaviors/input_behavior.hpp"

InputBehavior::InputBehavior(const IntentionSource &intentionSource)
    : intentionSource(intentionSource)
{
}

InputIntentions InputBehavior::decide(float, const ActorBehaviorContext &)
{
    return intentionSource.getIntentions();
}
