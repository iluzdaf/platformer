#include <cstddef>
#include <string_view>
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
#include "actor/behaviors/scripted_behavior.hpp"
#include "actor/behaviors/scripted_behavior_data.hpp"
#include "actor/behaviors/state_machine_behavior_data.hpp"
#include "actor/behaviors/senses_data.hpp"
#include "input/input_intentions.hpp"
#include "actor/actor_fact_rows.hpp"
#include "conditions/facts.hpp"
#include "state_machines/state_machine_data.hpp"

StateMachineBehavior::StateMachineBehavior(
    const StateMachineBehaviorData &data,
    std::optional<std::pair<glm::vec2, glm::vec2>> patrolBetween,
    const FactsData &declared,
    const SensesData &senses)
    : machine(data, actorRows(), declared)
{
    for (const TransitionData &transition : data.transitions)
        for (const auto &[name, asked] : transition.when)
            if (std::optional<std::string> why = whyNotSensed(name, senses))
                throw std::runtime_error(
                    "The transition from \"" + transition.from + "\" to \"" + transition.to +
                    "\" " + *why);

    for (const BehaviorStateData &state : data.states)
        steering.push_back(
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
                    else if constexpr (std::is_same_v<Does, ScriptedBehaviorData>)
                        return std::make_unique<ScriptedBehavior>(does);
                    else
                        return nullptr;
                },
                state.does));
}

ActorBehavior *StateMachineBehavior::steeringNow() const
{
    return steering.empty() ? nullptr : steering[machine.active()].get();
}

void StateMachineBehavior::reset()
{
    if (ActorBehavior *now = steeringNow())
        now->leave();

    for (const std::unique_ptr<ActorBehavior> &each : steering)
        if (each)
            each->reset();

    machine.reset();
}

InputIntentions StateMachineBehavior::decide(float deltaTime, const ActorFacts &context)
{
    static const FactsData nothingDeclared;
    ActorBehavior *was = steeringNow();
    std::optional<std::size_t> entered =
        machine.advance(deltaTime, context, context.facts ? *context.facts : nothingDeclared);
    if (entered && was)
        was->leave();
    if (entered && steering[*entered])
        steering[*entered]->reset();

    ActorBehavior *now = steeringNow();
    return now ? now->decide(deltaTime, context) : InputIntentions();
}

void StateMachineBehavior::scriptWith(StateScript *script)
{
    for (const std::unique_ptr<ActorBehavior> &each : steering)
        if (each)
            each->scriptWith(script);
}

std::string_view StateMachineBehavior::getStateName() const
{
    return machine.activeName();
}

std::optional<int> StateMachineBehavior::getCurrentNodeId() const
{
    ActorBehavior *now = steeringNow();
    return now ? now->getCurrentNodeId() : std::nullopt;
}

std::optional<int> StateMachineBehavior::getTargetNodeId() const
{
    ActorBehavior *now = steeringNow();
    return now ? now->getTargetNodeId() : std::nullopt;
}
