#pragma once

#include <optional>
#include <utility>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/actor_behavior.hpp"
#include "actor/behaviors/patrol_behavior_data.hpp"

class PatrolBehavior : public ActorBehavior
{
public:
    explicit PatrolBehavior(
        const PatrolBehaviorData &data,
        std::optional<std::pair<glm::vec2, glm::vec2>> patrolBetween = std::nullopt);
    void reset() override;
    InputIntentions decide(float deltaTime, const ActorBehaviorContext &context) override;
    std::optional<int> getCurrentNodeId() const;
    std::optional<int> getTargetNodeId() const;

private:
    PatrolBehaviorData data;

    // The two ends it walks between, and which one it is presently after.
    // Without them, the ends of the platform it finds itself on.
    std::optional<std::pair<glm::vec2, glm::vec2>> patrolBetween;
    bool headingForTheSecond = false;

    std::optional<int> currentNodeId, targetNodeId;
    std::vector<int> legsLeft;
    float jumpHeldFor = 0.0f;

    void anchor(const ActorBehaviorContext &context);
    void planRoute(const ActorBehaviorContext &context);
    std::optional<int> endOfTheBeat(const ActorBehaviorContext &context, bool second) const;
    bool hasArrived(const ActorBehaviorContext &context) const;
    bool hasLostTheRoute(const ActorBehaviorContext &context) const;
};
