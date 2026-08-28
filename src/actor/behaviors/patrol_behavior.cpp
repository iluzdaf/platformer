#include <algorithm>
#include <cmath>
#include <optional>
#include <glm/geometric.hpp>
#include <vector>
#include "actor/behaviors/patrol_behavior.hpp"
#include "navigation/navigation_graph.hpp"
#include "navigation/navigation_path.hpp"

namespace
{
    constexpr float SurfaceTolerance = 1.0f;

    // How close to the node a jump has to start. A step is under a pixel at the
    // speeds actors walk, so this is reachable rather than aspirational.
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

PatrolBehavior::PatrolBehavior(const PatrolBehaviorData &data)
    : data(data)
{
}

void PatrolBehavior::reset()
{
    currentNodeId.reset();
    targetNodeId.reset();
    legsLeft.clear();
    jumpHeldFor = 0.0f;
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

// Anchoring picks the nearest node, which is rarely the one being stood on, and
// a route can be lost outright by falling off it. Either way what the graph
// believes and where the actor is have parted company.
bool PatrolBehavior::hasLostTheRoute(const ActorBehaviorContext &context) const
{
    if (!currentNodeId || !context.onGround)
        return false;

    const NavigationGraph &navigationGraph = context.navigationGraph;
    NavigationNode node = navigationGraph.getNode(*currentNodeId);
    if (std::abs(node.position.y - context.worldPosition.y) > SurfaceTolerance)
        return true;

    // The same height is not the same platform. Another one level with this one
    // would otherwise pass for being where the route says the actor is, and it
    // would spend the rest of its life walking at a node it cannot reach.
    float reach = context.colliderSize.x * 0.5f + data.arrivalThreshold;
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

InputIntentions PatrolBehavior::decide(
    float deltaTime,
    const ActorBehaviorContext &context)
{
    InputIntentions inputIntentions;

    if (hasLostTheRoute(context))
    {
        currentNodeId.reset();
        targetNodeId.reset();
        legsLeft.clear();
        jumpHeldFor = 0.0f;
    }

    if (!currentNodeId)
        anchor(context);

    if (!currentNodeId)
        return inputIntentions;

    if (!targetNodeId)
        planRoute(context);

    if (!targetNodeId)
        return inputIntentions;

    if (hasArrived(context))
    {
        currentNodeId = targetNodeId;
        jumpHeldFor = 0.0f;

        legsLeft.erase(legsLeft.begin());
        if (legsLeft.empty())
            planRoute(context);
        else
            targetNodeId = legsLeft.front();

        if (!targetNodeId)
            return inputIntentions;
    }

    NavigationNode targetNode = context.navigationGraph.getNode(*targetNodeId);
    inputIntentions.direction.x = directionTowards(context.worldPosition.x, targetNode.position.x);

    const NavigationEdge *leg =
        edgeBetween(context.navigationGraph, *currentNodeId, *targetNodeId);

    // A jump that fell short puts the actor back on the ground with its hold
    // spent. Without giving that back it can neither jump again nor ever
    // arrive, and it walks at a node on another platform for good.
    if (leg && leg->type == EdgeType::Jump && context.onGround &&
        jumpHeldFor >= leg->holdDuration)
        jumpHeldFor = 0.0f;

    // A jump was simulated from the node, so it has to be made from the node,
    // and close enough to arrive is not close enough to jump. Being a few pixels
    // short is the difference between clearing a ledge and hitting its side.
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

    // A fall is a straight drop, and its landing sits a few pixels off the ledge
    // it leaves. Steering at it from the air overshoots by a step every frame,
    // so the actor arrives flicking from side to side.
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

std::optional<int> PatrolBehavior::getCurrentNodeId() const
{
    return currentNodeId;
}

std::optional<int> PatrolBehavior::getTargetNodeId() const
{
    return targetNodeId;
}

// Walk to the furthest node worth going to, then, on arrival, to the furthest
// from there, which is back the way it came.
void PatrolBehavior::planRoute(const ActorBehaviorContext &context)
{
    targetNodeId.reset();
    legsLeft.clear();

    if (!currentNodeId)
        return;

    const NavigationGraph &navigationGraph = context.navigationGraph;
    std::vector<int> worthGoingTo = data.roams
                                        ? roundTripFrom(navigationGraph, *currentNodeId)
                                        : walkableFrom(navigationGraph, *currentNodeId);

    glm::vec2 here = navigationGraph.getNode(*currentNodeId).position;

    std::optional<int> destination;
    float furthest = 0.0f;
    for (int id : worthGoingTo)
    {
        if (id == *currentNodeId)
            continue;

        float distance = glm::distance(navigationGraph.getNode(id).position, here);
        if (destination && distance <= furthest)
            continue;

        destination = id;
        furthest = distance;
    }

    if (!destination)
        return;

    std::vector<int> route = findPath(navigationGraph, *currentNodeId, *destination);
    if (route.size() < 2)
        return;

    legsLeft.assign(route.begin() + 1, route.end());
    targetNodeId = legsLeft.front();
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
