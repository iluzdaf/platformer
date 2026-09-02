#pragma once

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include <string_view>
#include <cstddef>
#include "actor/actor_behavior.hpp"
#include "actor/behaviors/state_machine_behavior_data.hpp"
#include "actor/actor_behavior_context.hpp"
#include "input/input_intentions.hpp"

class StateMachineBehavior : public ActorBehavior
{
public:
    explicit StateMachineBehavior(
        const StateMachineBehaviorData &data,
        std::optional<std::pair<glm::vec2, glm::vec2>> patrolBetween = std::nullopt);
    void reset() override;
    InputIntentions decide(float deltaTime, const ActorBehaviorContext &context) override;
    std::string_view getStateName() const override;
    std::optional<int> getCurrentNodeId() const override;
    std::optional<int> getTargetNodeId() const override;

private:
    StateMachineBehaviorData data;

    std::vector<std::unique_ptr<ActorBehavior>> states;
    std::size_t activeState = 0;
    std::vector<float> heldFor;

    std::optional<std::size_t> stateNamed(const std::string &name) const;
    void enter(std::size_t state);
    void takeATransition(float deltaTime, const ActorBehaviorContext &context);
};
