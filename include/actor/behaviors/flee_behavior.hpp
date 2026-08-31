#pragma once

#include <optional>
#include "actor/actor_behavior.hpp"
#include "actor/behaviors/flee_behavior_data.hpp"
#include "actor/behaviors/route_walker.hpp"
#include "actor/actor_behavior_context.hpp"
#include "input/input_intentions.hpp"

class FleeBehavior : public ActorBehavior
{
public:
    explicit FleeBehavior(const FleeBehaviorData &data);
    void reset() override;
    InputIntentions decide(float deltaTime, const ActorBehaviorContext &context) override;
    std::optional<int> getCurrentNodeId() const;
    std::optional<int> getTargetNodeId() const;

private:
    FleeBehaviorData data;
    RouteWalker walker;

    std::optional<int> furthestAlong(const ActorBehaviorContext &context, float away) const;
    std::optional<int> furthestFrom(const ActorBehaviorContext &context) const;
    bool fleeingTowardsTheThreat(const ActorBehaviorContext &context) const;
    void planRoute(const ActorBehaviorContext &context);
};
