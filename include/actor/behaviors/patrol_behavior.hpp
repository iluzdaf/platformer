#pragma once

#include <optional>
#include <vector>
#include "actor/actor_behavior.hpp"
#include "actor/behaviors/patrol_behavior_data.hpp"

class PatrolBehavior : public ActorBehavior
{
public:
    explicit PatrolBehavior(const PatrolBehaviorData &data);
    void reset() override;
    InputIntentions decide(float deltaTime, const ActorBehaviorContext &context) override;
    std::optional<int> getCurrentNodeId() const;
    std::optional<int> getTargetNodeId() const;

private:
    PatrolBehaviorData data;
    std::optional<int> currentNodeId, targetNodeId;
    std::vector<int> legsLeft;
    float jumpHeldFor = 0.0f;

    void anchor(const ActorBehaviorContext &context);
    void planRoute(const ActorBehaviorContext &context);
    bool hasArrived(const ActorBehaviorContext &context) const;
};
