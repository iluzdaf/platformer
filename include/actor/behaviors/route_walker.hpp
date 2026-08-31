#pragma once

#include <optional>
#include <vector>
#include "actor/actor_behavior_context.hpp"
#include "input/input_intentions.hpp"

class RouteWalker
{
public:
    explicit RouteWalker(float arrivalThreshold);
    void reset();
    void keepInStep(const ActorBehaviorContext &context);
    bool isAnchored() const;
    void advanceOnArrival(const ActorBehaviorContext &context);
    bool routeFinished() const;
    void takeRouteTo(
        const ActorBehaviorContext &context,
        int destinationNodeId,
        std::optional<float> stopShortAt = std::nullopt);
    InputIntentions follow(float deltaTime, const ActorBehaviorContext &context);
    std::optional<int> getCurrentNodeId() const;
    std::optional<int> getTargetNodeId() const;

private:
    float arrivalThreshold;

    std::optional<int> currentNodeId, targetNodeId;
    std::optional<float> stopShortAt;
    std::vector<int> legsLeft;
    float jumpHeldFor = 0.0f;

    void anchor(const ActorBehaviorContext &context);
    float targetX(const ActorBehaviorContext &context) const;
    bool hasArrived(const ActorBehaviorContext &context) const;
    bool withinReachOf(const ActorBehaviorContext &context, int nodeId) const;
    bool hasLostTheRoute(const ActorBehaviorContext &context) const;
};
