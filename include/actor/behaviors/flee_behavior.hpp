#pragma once

#include <optional>
#include "actor/actor_behavior.hpp"
#include "actor/behaviors/flee_behavior_data.hpp"
#include "actor/behaviors/route_walker.hpp"
#include "actor/actor_facts.hpp"
#include "input/input_intentions.hpp"

class FleeBehavior : public ActorBehavior
{
public:
    explicit FleeBehavior(const FleeBehaviorData &data);
    void reset() override;
    InputIntentions decide(float deltaTime, const ActorFacts &context) override;
    std::optional<int> getCurrentNodeId() const override;
    std::optional<int> getTargetNodeId() const override;

private:
    FleeBehaviorData data;
    RouteWalker walker;

    std::optional<int> furthestFrom(const ActorFacts &context) const;
    bool fleeingTowardsTheThreat(const ActorFacts &context) const;
    void planRoute(const ActorFacts &context);
};
