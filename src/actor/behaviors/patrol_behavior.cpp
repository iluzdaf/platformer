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

PatrolBehavior::PatrolBehavior(
    const PatrolBehaviorData &data,
    std::optional<std::pair<glm::vec2, glm::vec2>> patrolBetween)
    : data(data),
      patrolBetween(patrolBetween)
{
}

std::optional<int> PatrolBehavior::endOfTheBeat(
    const ActorBehaviorContext &context,
    bool second) const
{
    const NavigationGraph &navigationGraph = context.navigationGraph;

    if (patrolBetween)
    {
        glm::vec2 end = second ? patrolBetween->second : patrolBetween->first;
        std::optional<int> nearest;
        float nearestDistance = 0.0f;
        for (const auto &[id, node] : navigationGraph.getNodes())
        {
            float distance = glm::distance(node.position, end);
            if (nearest && distance >= nearestDistance)
                continue;

            nearest = id;
            nearestDistance = distance;
        }

        return nearest;
    }

    if (!currentNodeId)
        return std::nullopt;

    std::optional<int> end;
    for (int id : walkableFrom(navigationGraph, *currentNodeId))
    {
        if (!end)
        {
            end = id;
            continue;
        }

        float here = navigationGraph.getNode(id).position.x;
        float best = navigationGraph.getNode(*end).position.x;
        if (second ? here > best : here < best)
            end = id;
    }

    return end;
}

void PatrolBehavior::reset()
{
    currentNodeId.reset();
    targetNodeId.reset();
    legsLeft.clear();
    headingForTheSecond = false;
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

bool PatrolBehavior::hasLostTheRoute(const ActorBehaviorContext &context) const
{
    if (!currentNodeId || !context.onGround)
        return false;

    const NavigationGraph &navigationGraph = context.navigationGraph;
    NavigationNode node = navigationGraph.getNode(*currentNodeId);
    if (std::abs(node.position.y - context.worldPosition.y) > SurfaceTolerance)
        return true;

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

std::optional<int> PatrolBehavior::getCurrentNodeId() const
{
    return currentNodeId;
}

std::optional<int> PatrolBehavior::getTargetNodeId() const
{
    return targetNodeId;
}

void PatrolBehavior::planRoute(const ActorBehaviorContext &context)
{
    targetNodeId.reset();
    legsLeft.clear();

    if (!currentNodeId)
        return;

    std::optional<int> destination = endOfTheBeat(context, headingForTheSecond);

    if (destination && *destination == *currentNodeId)
    {
        headingForTheSecond = !headingForTheSecond;
        destination = endOfTheBeat(context, headingForTheSecond);
    }

    if (!destination || *destination == *currentNodeId)
        return;

    std::vector<int> route = findPath(context.navigationGraph, *currentNodeId, *destination);
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
