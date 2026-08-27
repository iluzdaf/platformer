#include <cmath>
#include <optional>
#include "actor/behaviors/patrol_behavior.hpp"
#include "navigation/navigation_graph.hpp"

namespace
{
    constexpr float SurfaceTolerance = 1.0f;

    float directionTowards(float from, float to)
    {
        float delta = to - from;
        return delta > 0.0f ? 1.0f : (delta < 0.0f ? -1.0f : 0.0f);
    }

    bool hasSomewhereToGo(const NavigationGraph &navigationGraph, int nodeId)
    {
        return !navigationGraph.getOutgoingEdges(nodeId).empty();
    }

    std::optional<EdgeType> edgeTypeBetween(
        const NavigationGraph &navigationGraph,
        int fromId,
        int toId)
    {
        for (const auto &edge : navigationGraph.getOutgoingEdges(fromId))
            if (edge.toId == toId)
                return edge.type;

        return std::nullopt;
    }
}

PatrolBehavior::PatrolBehavior(const PatrolBehaviorData &data)
    : data(data)
{
}

void PatrolBehavior::reset()
{
    currentNodeId.reset();
    targetNodeId.reset();
    previousNodeId.reset();
}

void PatrolBehavior::anchor(const ActorBehaviorContext &context)
{
    float nearestDrop = 0.0f;
    float nearestDistance = 0.0f;
    for (const auto &[id, node] : context.navigationGraph.getNodes())
    {
        if (!hasSomewhereToGo(context.navigationGraph, id))
            continue;

        float drop = node.position.y - context.worldPosition.y;
        if (drop < -SurfaceTolerance)
            continue;

        float distance = std::abs(node.position.x - context.worldPosition.x);
        bool nearer = !currentNodeId ||
                      drop < nearestDrop ||
                      (drop == nearestDrop && distance < nearestDistance);
        if (!nearer)
            continue;

        currentNodeId = id;
        nearestDrop = drop;
        nearestDistance = distance;
    }
}

InputIntentions PatrolBehavior::decide(
    float,
    const ActorBehaviorContext &context)
{
    InputIntentions inputIntentions;

    if (!currentNodeId)
        anchor(context);

    if (!currentNodeId)
        return inputIntentions;

    if (!targetNodeId)
        pickTarget(context);

    if (!targetNodeId)
        return inputIntentions;

    if (hasArrived(context))
    {
        previousNodeId = currentNodeId;
        currentNodeId = targetNodeId;
        targetNodeId.reset();
        pickTarget(context);

        if (!targetNodeId)
            return inputIntentions;
    }

    NavigationNode targetNode = context.navigationGraph.getNode(*targetNodeId);
    inputIntentions.direction.x = directionTowards(context.worldPosition.x, targetNode.position.x);

    if (edgeTypeBetween(context.navigationGraph, *currentNodeId, *targetNodeId) == EdgeType::Jump)
    {
        inputIntentions.jumpRequested = true;
        inputIntentions.jumpHeld = true;
    }

    return inputIntentions;
}

std::optional<int> PatrolBehavior::getCurrentNodeId() const
{
    return currentNodeId;
}

std::optional<int> PatrolBehavior::getTargetNodeId() const
{
    return targetNodeId;
}

void PatrolBehavior::pickTarget(const ActorBehaviorContext &context)
{
    targetNodeId.reset();

    if (!currentNodeId)
        return;

    std::optional<int> wayBack;
    for (const auto &edge : context.navigationGraph.getOutgoingEdges(*currentNodeId))
    {
        if (previousNodeId && edge.toId == *previousNodeId)
        {
            if (!wayBack || edge.toId < *wayBack)
                wayBack = edge.toId;
            continue;
        }

        if (!targetNodeId || edge.toId < *targetNodeId)
            targetNodeId = edge.toId;
    }

    if (!targetNodeId)
        targetNodeId = wayBack;
}

bool PatrolBehavior::hasArrived(const ActorBehaviorContext &context) const
{
    if (!currentNodeId || !targetNodeId)
        return false;

    const NavigationGraph &navigationGraph = context.navigationGraph;
    NavigationNode targetNode = navigationGraph.getNode(*targetNodeId);

    if (std::abs(targetNode.position.y - context.worldPosition.y) > SurfaceTolerance)
        return false;

    float reach = context.colliderSize.x * 0.5f + data.arrivalThreshold;
    if (std::abs(targetNode.position.x - context.worldPosition.x) <= reach)
        return true;

    float legDirection = directionTowards(
        navigationGraph.getNode(*currentNodeId).position.x,
        targetNode.position.x);

    return directionTowards(context.worldPosition.x, targetNode.position.x) != legDirection;
}
