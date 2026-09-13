#pragma once

#include <memory>
#include <optional>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include <string_view>
#include "actor/behaviors/senses_data.hpp"
#include "actor/actor_behavior.hpp"
#include "actor/behaviors/state_machine_behavior_data.hpp"
#include "actor/actor_facts.hpp"
#include "conditions/facts.hpp"
#include "input/input_intentions.hpp"
#include "state_machines/state_machine.hpp"

class StateMachineBehavior : public ActorBehavior
{
public:
    explicit StateMachineBehavior(
        const StateMachineBehaviorData &data,
        const FactsData &declared = FactsData{},
        const SensesData &senses = SensesData{});
    void reset() override;
    void scriptWith(StateScript *script) override;
    InputIntentions decide(float deltaTime, const ActorFacts &context) override;
    std::string_view getStateName() const override;
    std::optional<int> getCurrentNodeId() const override;
    std::optional<int> getTargetNodeId() const override;

private:
    StateMachine<ActorFacts> machine;
    std::vector<std::unique_ptr<ActorBehavior>> steering;

    ActorBehavior *steeringNow() const;
};
