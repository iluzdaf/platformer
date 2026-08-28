#include <glm/geometric.hpp>
#include "actor/behaviors/flee_behavior.hpp"
#include "navigation/navigation_graph.hpp"
#include "navigation/navigation_path.hpp"

FleeBehavior::FleeBehavior(const FleeBehaviorData &data) : data(data), walker(data.arrivalThreshold)
{
}

void FleeBehavior::reset()
{
    walker.reset();
}

std::optional<int> FleeBehavior::furthestAlong(const ActorBehaviorContext &context, float away)
    const
{
    const NavigationGraph &navigationGraph = context.navigationGraph;

    std::optional<int> furthest;
    float furthestDistance = 0.0f;
    for (int id : roundTripFrom(navigationGraph, *walker.getCurrentNodeId()))
    {
        float distance =
            (navigationGraph.getNode(id).position.x - context.threatPosition->x) * away;
        if (furthest && distance <= furthestDistance)
            continue;

        furthest = id;
        furthestDistance = distance;
    }

    return furthest;
}

std::optional<int> FleeBehavior::furthestFrom(const ActorBehaviorContext &context) const
{
    std::optional<int> from = walker.getCurrentNodeId();
    if (!from || !context.threatPosition)
        return std::nullopt;

    float away = context.worldPosition.x < context.threatPosition->x ? -1.0f : 1.0f;
    std::optional<int> refuge = furthestAlong(context, away);

    bool cornered = refuge && *refuge == *from;
    if (cornered &&
        glm::distance(context.worldPosition, *context.threatPosition) <= data.breakPastWithin)
        return furthestAlong(context, -away);

    return refuge;
}

bool FleeBehavior::fleeingTowardsTheThreat(const ActorBehaviorContext &context) const
{
    std::optional<int> destination = walker.getTargetNodeId();
    if (!destination || !context.threatPosition || !context.onGround)
        return false;

    glm::vec2 refuge = context.navigationGraph.getNode(*destination).position;

    return glm::distance(refuge, *context.threatPosition) <
           glm::distance(context.worldPosition, *context.threatPosition);
}

void FleeBehavior::planRoute(const ActorBehaviorContext &context)
{
    std::optional<int> refuge = furthestFrom(context);
    if (!refuge)
        return;

    walker.takeRouteTo(context, *refuge);
}

InputIntentions FleeBehavior::decide(float deltaTime, const ActorBehaviorContext &context)
{
    walker.keepInStep(context);
    if (!walker.isAnchored())
        return InputIntentions();

    walker.advanceOnArrival(context);
    if (walker.routeFinished() || fleeingTowardsTheThreat(context))
        planRoute(context);

    return walker.follow(deltaTime, context);
}

std::optional<int> FleeBehavior::getCurrentNodeId() const
{
    return walker.getCurrentNodeId();
}

std::optional<int> FleeBehavior::getTargetNodeId() const
{
    return walker.getTargetNodeId();
}
