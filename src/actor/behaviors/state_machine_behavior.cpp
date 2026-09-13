#include <string_view>
#include <cstddef>
#include <glm/geometric.hpp>
#include <optional>
#include <stdexcept>
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
#include "actor/actor_facts.hpp"
#include "actor/behaviors/chase_behavior.hpp"
#include "actor/behaviors/flee_behavior.hpp"
#include "actor/behaviors/patrol_behavior.hpp"
#include "actor/behaviors/attack_behavior.hpp"
#include "actor/behaviors/state_machine_behavior_data.hpp"
#include "actor/behaviors/senses_data.hpp"
#include "input/input_intentions.hpp"
#include "actor/actor_fact_rows.hpp"
#include "conditions/asked.hpp"
#include "conditions/fact_rows.hpp"
#include "conditions/facts.hpp"

namespace
{
    std::optional<AskedKind> kindKnown(const std::string &name, const FactsData &declared)
    {
        if (const FactRow<ActorFacts> *row = rowNamed(actorRows(), name))
            return row->kind;

        auto fact = declared.find(name);
        if (fact != declared.end())
            return kindOf(fact->second);

        return std::nullopt;
    }

    bool conditionHolds(const BehaviorTransitionData &transition, const ActorFacts &context)
    {
        for (const auto &[name, asked] : transition.when)
        {
            if (const FactRow<ActorFacts> *row = rowNamed(actorRows(), name))
            {
                if (!row->holds(asked, context))
                    return false;

                continue;
            }

            auto fact = context.facts ? context.facts->find(name) : FactsData::const_iterator{};
            if (!context.facts || fact == context.facts->end())
                throw std::runtime_error(
                    "A condition asks about \"" + name + "\", and there is no such fact");

            if (fact->second != asked)
                return false;
        }

        return true;
    }
}

StateMachineBehavior::StateMachineBehavior(
    const StateMachineBehaviorData &data,
    std::optional<std::pair<glm::vec2, glm::vec2>> patrolBetween,
    const FactsData &declared,
    const SensesData &senses)
    : data(data), heldFor(data.transitions.size(), 0.0f), sinceLeft(data.states.size(), 1e9f)
{
    for (const BehaviorTransitionData &transition : this->data.transitions)
        for (const auto &[name, asked] : transition.when)
        {
            std::optional<std::string> why = whyNotAsked(name, asked, kindKnown(name, declared));
            if (!why)
                why = whyNotSensed(name, senses);
            if (why)
                throw std::runtime_error(
                    "The transition from \"" + transition.from + "\" to \"" + transition.to +
                    "\" " + *why);
        }

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

void StateMachineBehavior::takeATransition(float deltaTime, const ActorFacts &context)
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

InputIntentions StateMachineBehavior::decide(float deltaTime, const ActorFacts &context)
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
