#include <string_view>
#include <cstddef>
#include <glm/geometric.hpp>
#include <optional>
#include <utility>
#include <memory>
#include <string>
#include "actor/behaviors/state_machine_behavior.hpp"
#include "actor/actor_behavior_context.hpp"
#include "actor/behaviors/flee_behavior.hpp"
#include "actor/behaviors/patrol_behavior.hpp"
#include "actor/behaviors/state_machine_behavior_data.hpp"
#include "input/input_intentions.hpp"
#include "navigation/navigation_path.hpp"

namespace
{
    bool conditionHolds(
        const BehaviorTransitionData &transition,
        const ActorBehaviorContext &context)
    {
        if (transition.threatWithin)
        {
            if (!context.threatPosition)
                return false;

            if (glm::distance(context.worldPosition, *context.threatPosition) >
                *transition.threatWithin)
                return false;
        }

        if (transition.threatBeyond && context.threatPosition)
        {
            if (glm::distance(context.worldPosition, *context.threatPosition) <=
                *transition.threatBeyond)
                return false;
        }

        if (transition.threatOnMySurface)
        {
            bool sharing =
                context.threatPosition &&
                onTheSameRun(
                    context.navigationGraph, context.worldPosition, *context.threatPosition);

            if (sharing != *transition.threatOnMySurface)
                return false;
        }

        return true;
    }
}

StateMachineBehavior::StateMachineBehavior(
    const StateMachineBehaviorData &data,
    std::optional<std::pair<glm::vec2, glm::vec2>> patrolBetween)
    : data(data), heldFor(data.transitions.size(), 0.0f)
{
    for (const BehaviorStateData &state : this->data.states)
    {
        if (state.patrolBehaviorData)
            states.push_back(
                std::make_unique<PatrolBehavior>(*state.patrolBehaviorData, patrolBetween));
        else if (state.fleeBehaviorData)
            states.push_back(std::make_unique<FleeBehavior>(*state.fleeBehaviorData));
        else
            states.push_back(nullptr);
    }
}

std::optional<std::size_t> StateMachineBehavior::stateNamed(const std::string &name) const
{
    for (std::size_t state = 0; state < data.states.size(); ++state)
        if (data.states[state].name == name)
            return state;

    return std::nullopt;
}

void StateMachineBehavior::enter(std::size_t state)
{
    activeState = state;
    heldFor.assign(data.transitions.size(), 0.0f);

    if (states[state])
        states[state]->reset();
}

void StateMachineBehavior::reset()
{
    if (states.empty())
        return;

    for (const std::unique_ptr<ActorBehavior> &state : states)
        if (state)
            state->reset();

    enter(0);
}

void StateMachineBehavior::takeATransition(float deltaTime, const ActorBehaviorContext &context)
{
    for (std::size_t index = 0; index < data.transitions.size(); ++index)
    {
        const BehaviorTransitionData &transition = data.transitions[index];
        if (transition.from != data.states[activeState].name)
            continue;

        if (!conditionHolds(transition, context))
        {
            heldFor[index] = 0.0f;
            continue;
        }

        heldFor[index] += deltaTime;
        if (heldFor[index] < transition.after)
            continue;

        std::optional<std::size_t> destination = stateNamed(transition.to);
        if (!destination || *destination == activeState)
            continue;

        enter(*destination);
        return;
    }
}

InputIntentions StateMachineBehavior::decide(float deltaTime, const ActorBehaviorContext &context)
{
    if (states.empty())
        return InputIntentions();

    takeATransition(deltaTime, context);

    if (!states[activeState])
        return InputIntentions();

    return states[activeState]->decide(deltaTime, context);
}

std::string_view StateMachineBehavior::getStateName() const
{
    return states.empty() ? std::string_view{} : data.states[activeState].name;
}

std::optional<int> StateMachineBehavior::getCurrentNodeId() const
{
    return states.empty() ? std::nullopt : states[activeState]->getCurrentNodeId();
}

std::optional<int> StateMachineBehavior::getTargetNodeId() const
{
    return states.empty() ? std::nullopt : states[activeState]->getTargetNodeId();
}
