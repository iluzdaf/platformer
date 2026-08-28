#include <algorithm>
#include <cmath>
#include "actor/behaviors/route_walker.hpp"
#include "navigation/navigation_graph.hpp"
#include "navigation/navigation_path.hpp"

namespace
{
    constexpr float SurfaceTolerance = 1.0f;

    constexpr float TakeOffReach = 1.5f;

    float directionTowards(float from, float to)
    {
        float delta = to - from;
        return delta > 0.0f ? 1.0f : (delta < 0.0f ? -1.0f : 0.0f);
    }

    bool hasSomewhereToGo(const NavigationGraph &navigationGraph, int nodeId)
    {
        return !navigationGraph.getOutgoingEdges(nodeId).empty();
    }

    const NavigationEdge *edgeBetween(
        const NavigationGraph &navigationGraph,
        int fromId,
        int toId)
    {
        for (const auto &edge : navigationGraph.getOutgoingEdges(fromId))
            if (edge.toId == toId)
                return &edge;

        return nullptr;
    }
}

RouteWalker::RouteWalker(float arrivalThreshold)
    : arrivalThreshold(arrivalThreshold)
{
}

void RouteWalker::reset()
{
    currentNodeId.reset();
    targetNodeId.reset();
    legsLeft.clear();
    jumpHeldFor = 0.0f;
}

void RouteWalker::anchor(const ActorBehaviorContext &context)
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

bool RouteWalker::hasLostTheRoute(const ActorBehaviorContext &context) const
{
    if (!currentNodeId || !context.onGround)
        return false;

    const NavigationGraph &navigationGraph = context.navigationGraph;
    NavigationNode node = navigationGraph.getNode(*currentNodeId);
    if (std::abs(node.position.y - context.worldPosition.y) > SurfaceTolerance)
        return true;

    float reach = context.colliderSize.x * 0.5f + arrivalThreshold;
    float leftEnd = node.position.x;
    float rightEnd = node.position.x;
    for (int id : walkableFrom(navigationGraph, *currentNodeId))
    {
        NavigationNode onFoot = navigationGraph.getNode(id);
        if (std::abs(onFoot.position.y - node.position.y) > SurfaceTolerance)
            continue;

        leftEnd = std::min(leftEnd, onFoot.position.x);
        rightEnd = std::max(rightEnd, onFoot.position.x);
    }

    return context.worldPosition.x < leftEnd - reach || context.worldPosition.x > rightEnd + reach;
}

void RouteWalker::keepInStep(const ActorBehaviorContext &context)
{
    if (hasLostTheRoute(context))
        reset();

    if (!currentNodeId)
        anchor(context);
}

bool RouteWalker::isAnchored() const
{
    return currentNodeId.has_value();
}

bool RouteWalker::routeFinished() const
{
    return !targetNodeId;
}

void RouteWalker::advanceOnArrival(const ActorBehaviorContext &context)
{
    if (!targetNodeId || !hasArrived(context))
        return;

    currentNodeId = targetNodeId;
    jumpHeldFor = 0.0f;

    legsLeft.erase(legsLeft.begin());
    targetNodeId = legsLeft.empty() ? std::nullopt : std::optional(legsLeft.front());
}

void RouteWalker::takeRouteTo(const ActorBehaviorContext &context, int destinationNodeId)
{
    targetNodeId.reset();
    legsLeft.clear();

    if (!currentNodeId || destinationNodeId == *currentNodeId)
        return;

    std::vector<int> route = findPath(context.navigationGraph, *currentNodeId, destinationNodeId);
    if (route.size() < 2)
        return;

    legsLeft.assign(route.begin() + 1, route.end());
    targetNodeId = legsLeft.front();
}

InputIntentions RouteWalker::follow(float deltaTime, const ActorBehaviorContext &context)
{
    InputIntentions inputIntentions;

    if (!currentNodeId || !targetNodeId)
        return inputIntentions;

    NavigationNode targetNode = context.navigationGraph.getNode(*targetNodeId);
    inputIntentions.direction.x = directionTowards(context.worldPosition.x, targetNode.position.x);

    const NavigationEdge *leg =
        edgeBetween(context.navigationGraph, *currentNodeId, *targetNodeId);

    if (leg && leg->type == EdgeType::Jump && context.onGround &&
        jumpHeldFor >= leg->holdDuration)
        jumpHeldFor = 0.0f;

    if (leg && leg->type == EdgeType::Jump && jumpHeldFor == 0.0f)
    {
        NavigationNode takeOff = context.navigationGraph.getNode(*currentNodeId);
        if (std::abs(takeOff.position.x - context.worldPosition.x) > TakeOffReach)
        {
            inputIntentions.direction.x =
                directionTowards(context.worldPosition.x, takeOff.position.x);
            return inputIntentions;
        }
    }

    if (leg && leg->type == EdgeType::Fall && !context.onGround)
        inputIntentions.direction.x = 0.0f;

    if (leg && leg->type == EdgeType::Jump && jumpHeldFor < leg->holdDuration)
    {
        jumpHeldFor += deltaTime;
        inputIntentions.jumpRequested = true;
        inputIntentions.jumpHeld = true;
    }

    return inputIntentions;
}

bool RouteWalker::hasArrived(const ActorBehaviorContext &context) const
{
    if (!currentNodeId || !targetNodeId)
        return false;

    const NavigationGraph &navigationGraph = context.navigationGraph;
    NavigationNode targetNode = navigationGraph.getNode(*targetNodeId);

    if (std::abs(targetNode.position.y - context.worldPosition.y) > SurfaceTolerance)
        return false;

    float reach = context.colliderSize.x * 0.5f + arrivalThreshold;
    if (std::abs(targetNode.position.x - context.worldPosition.x) <= reach)
        return true;

    float legDirection = directionTowards(
        navigationGraph.getNode(*currentNodeId).position.x,
        targetNode.position.x);

    return directionTowards(context.worldPosition.x, targetNode.position.x) != legDirection;
}

std::optional<int> RouteWalker::getCurrentNodeId() const
{
    return currentNodeId;
}

std::optional<int> RouteWalker::getTargetNodeId() const
{
    return targetNodeId;
}
