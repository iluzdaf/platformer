#include <glm/geometric.hpp>
#include "actor/behaviors/flee_behavior.hpp"
#include "navigation/navigation_graph.hpp"
#include "navigation/navigation_path.hpp"

FleeBehavior::FleeBehavior(const FleeBehaviorData &data)
    : data(data),
      walker(data.arrivalThreshold)
{
}

void FleeBehavior::reset()
{
    walker.reset();
}

std::optional<int> FleeBehavior::furthestFrom(const ActorBehaviorContext &context) const
{
    std::optional<int> from = walker.getCurrentNodeId();
    if (!from || !context.threatPosition)
        return std::nullopt;

    const NavigationGraph &navigationGraph = context.navigationGraph;
    float away = context.worldPosition.x - context.threatPosition->x;

    std::optional<int> furthest;
    float furthestDistance = 0.0f;
    for (int id : roundTripFrom(navigationGraph, *from))
    {
        glm::vec2 refuge = navigationGraph.getNode(id).position;
        if (away != 0.0f && (refuge.x - context.threatPosition->x) * away < 0.0f)
            continue;

        float distance = glm::distance(refuge, *context.threatPosition);
        if (furthest && distance <= furthestDistance)
            continue;

        furthest = id;
        furthestDistance = distance;
    }

    return furthest;
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

InputIntentions FleeBehavior::decide(
    float deltaTime,
    const ActorBehaviorContext &context)
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
