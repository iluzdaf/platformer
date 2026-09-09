#include <string_view>
#include <cstddef>
#include <glm/geometric.hpp>
#include <optional>
#include <utility>
#include <memory>
#include <type_traits>
#include <variant>
#include <string>
#include "actor/behaviors/state_machine_behavior.hpp"
#include "actor/actor_behavior.hpp"
#include "actor/behaviors/attack_behavior_data.hpp"
#include "actor/behaviors/chase_behavior_data.hpp"
#include "actor/behaviors/flee_behavior_data.hpp"
#include "actor/behaviors/patrol_behavior_data.hpp"
#include "actor/actor_behavior_context.hpp"
#include "actor/behaviors/chase_behavior.hpp"
#include "actor/behaviors/flee_behavior.hpp"
#include "actor/behaviors/patrol_behavior.hpp"
#include "actor/behaviors/attack_behavior.hpp"
#include "actor/behaviors/state_machine_behavior_data.hpp"
#include "input/input_intentions.hpp"
#include "navigation/navigation_place.hpp"

namespace
{
    bool conditionHolds(
        const BehaviorTransitionData &transition,
        const ActorBehaviorContext &context)
    {
        if (transition.threatWithin)
        {
            if (!context.threatFeet)
                return false;

            if (glm::distance(context.feet, *context.threatFeet) > *transition.threatWithin)
                return false;
        }

        if (transition.threatBeyond && context.threatFeet)
        {
            if (glm::distance(context.feet, *context.threatFeet) <= *transition.threatBeyond)
                return false;
        }

        if (transition.onGround && context.contacts.onGround != *transition.onGround)
            return false;

        if (transition.cornered)
        {
            bool cornered = context.threatFeet && corneredBy(
                                                      context.navigationGraph,
                                                      context.feet,
                                                      *context.threatFeet,
                                                      context.colliderSize.x);
            if (cornered != *transition.cornered)
                return false;
        }

        if (transition.threatOnMySurface)
        {
            bool sharing = context.threatFeet &&
                           onTheSameRun(context.navigationGraph, context.feet, *context.threatFeet);

            if (sharing != *transition.threatOnMySurface)
                return false;
        }

        return true;
    }
}

StateMachineBehavior::StateMachineBehavior(
    const StateMachineBehaviorData &data,
    std::optional<std::pair<glm::vec2, glm::vec2>> patrolBetween)
    : data(data), heldFor(data.transitions.size(), 0.0f), sinceLeft(data.states.size(), 1e9f)
{
    for (const BehaviorStateData &state : this->data.states)
        states.push_back(
            std::visit(
                [&patrolBetween](const auto &does) -> std::unique_ptr<ActorBehavior>
                {
                    using Does = std::remove_cvref_t<decltype(does)>;
                    if constexpr (std::is_same_v<Does, PatrolBehaviorData>)
                        return std::make_unique<PatrolBehavior>(does, patrolBetween);
                    else if constexpr (std::is_same_v<Does, FleeBehaviorData>)
                        return std::make_unique<FleeBehavior>(does);
                    else if constexpr (std::is_same_v<Does, ChaseBehaviorData>)
                        return std::make_unique<ChaseBehavior>(does);
                    else if constexpr (std::is_same_v<Does, AttackBehaviorData>)
                        return std::make_unique<AttackBehavior>(does);
                    else
                        return nullptr;
                },
                state.does));
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
    sinceLeft[activeState] = 0.0f;
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

        if (sinceLeft[*destination] < data.states[*destination].cooldown)
            continue;

        enter(*destination);
        return;
    }
}

InputIntentions StateMachineBehavior::decide(float deltaTime, const ActorBehaviorContext &context)
{
    if (states.empty())
        return InputIntentions();

    for (float &since : sinceLeft)
        since += deltaTime;

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
    return states.empty() || !states[activeState] ? std::nullopt
                                                  : states[activeState]->getCurrentNodeId();
}

std::optional<int> StateMachineBehavior::getTargetNodeId() const
{
    return states.empty() || !states[activeState] ? std::nullopt
                                                  : states[activeState]->getTargetNodeId();
}

float StateMachineBehavior::secondsSinceLeaving(std::string_view state) const
{
    std::optional<std::size_t> which = stateNamed(std::string(state));
    return which ? sinceLeft[*which] : 0.0f;
}
