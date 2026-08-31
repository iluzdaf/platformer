#pragma once

#include <optional>
#include <utility>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/actor_behavior.hpp"
#include "actor/behaviors/patrol_behavior_data.hpp"
#include "actor/behaviors/route_walker.hpp"
#include "actor/actor_behavior_context.hpp"
#include "input/input_intentions.hpp"

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
    struct BeatEnd
    {
        glm::vec2 position = glm::vec2(0.0f);
        int routeVia = 0;
    };

    PatrolBehaviorData data;
    RouteWalker walker;

    std::optional<std::pair<glm::vec2, glm::vec2>> patrolBetween;
    bool headingForTheSecond = false;

    void planRoute(const ActorBehaviorContext &context);
    std::optional<BeatEnd> endOfTheBeat(const ActorBehaviorContext &context, bool second) const;
    bool standingAt(const ActorBehaviorContext &context, const BeatEnd &end) const;
};
