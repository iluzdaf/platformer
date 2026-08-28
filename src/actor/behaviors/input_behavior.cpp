#include "actor/behaviors/input_behavior.hpp"
#include "input/intention_source.hpp"
#include "input/input_intentions.hpp"
#include "actor/actor_behavior_context.hpp"

InputBehavior::InputBehavior(const IntentionSource &intentionSource)
    : intentionSource(intentionSource)
{
}

InputIntentions InputBehavior::decide(float, const ActorBehaviorContext &)
{
    return intentionSource.getIntentions();
}
