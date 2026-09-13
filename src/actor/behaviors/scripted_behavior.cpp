#include <optional>
#include <stdexcept>
#include "actor/behaviors/scripted_behavior.hpp"
#include "actor/behaviors/scripted_behavior_data.hpp"
#include "actor/behaviors/state_script.hpp"
#include "actor/actor_facts.hpp"
#include "input/input_intentions.hpp"

ScriptedBehavior::ScriptedBehavior(const ScriptedBehaviorData &data)
    : call(data.call), walker(data.arrivalThreshold)
{
    if (call.empty())
        throw std::runtime_error("A scripted state must say what it calls");
}

void ScriptedBehavior::scriptWith(StateScript *newScript)
{
    script = newScript;
}

void ScriptedBehavior::reset()
{
    walker.reset();
    entered = false;
}

void ScriptedBehavior::leave()
{
    if (entered && script)
        script->exit(call);

    entered = false;
}

InputIntentions ScriptedBehavior::decide(float deltaTime, const ActorFacts &context)
{
    if (!script)
        return InputIntentions();

    walker.keepInStep(context);
    walker.advanceOnArrival(context);
    if (!entered)
    {
        entered = true;
        script->enter(call);
    }

    return script->decide(call, walker, context, deltaTime);
}

std::optional<int> ScriptedBehavior::getCurrentNodeId() const
{
    return walker.getCurrentNodeId();
}

std::optional<int> ScriptedBehavior::getTargetNodeId() const
{
    return walker.getTargetNodeId();
}
