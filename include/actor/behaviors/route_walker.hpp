#pragma once

#include <optional>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include "actor/actor_facts.hpp"
#include "input/input_intentions.hpp"

class RouteWalker
{
public:
    explicit RouteWalker(float arrivalThreshold = 2.0f);
    void reset();
    void keepInStep(const ActorFacts &context);
    bool isAnchored() const;
    void advanceOnArrival(const ActorFacts &context);
    bool routeFinished() const;
    void takeRouteTo(
        const ActorFacts &context,
        int destinationNodeId,
        std::optional<glm::vec2> stopShortAt = std::nullopt);
    InputIntentions follow(float deltaTime, const ActorFacts &context);
    std::optional<int> getCurrentNodeId() const;
    std::optional<int> getTargetNodeId() const;

private:
    float arrivalThreshold;

    std::optional<int> currentNodeId, targetNodeId;
    std::optional<glm::vec2> stopShortAt;
    std::vector<int> legsLeft;
    float jumpHeldFor = 0.0f;

    void anchor(const ActorFacts &context);
    glm::vec2 targetPosition(const ActorFacts &context, int setOffAt, int headingFor) const;
    bool hasArrived(const ActorFacts &context, int setOffAt, int headingFor) const;
    bool withinReachOf(const ActorFacts &context, int nodeId) const;
    bool hasLostTheRoute(const ActorFacts &context) const;
    bool walksGroundThatIsGone(const NavigationGraph &navigationGraph) const;
};
