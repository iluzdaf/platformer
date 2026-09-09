#pragma once

#include <optional>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/actor_behavior.hpp"
#include "actor/behaviors/chase_behavior_data.hpp"
#include "actor/behaviors/route_walker.hpp"
#include "actor/actor_behavior_context.hpp"
#include "input/input_intentions.hpp"

class ChaseBehavior : public ActorBehavior
{
public:
    explicit ChaseBehavior(const ChaseBehaviorData &data);
    void reset() override;
    InputIntentions decide(float deltaTime, const ActorBehaviorContext &context) override;
    std::optional<int> getCurrentNodeId() const override;
    std::optional<int> getTargetNodeId() const override;

private:
    ChaseBehaviorData data;
    RouteWalker walker;
    std::optional<glm::vec2> lastSeenAt;

    bool caughtUp(const ActorBehaviorContext &context) const;
    bool threatHasMoved(const ActorBehaviorContext &context) const;
    std::optional<int> whereToCloseIn(const ActorBehaviorContext &context) const;
    void planRoute(const ActorBehaviorContext &context);
};
