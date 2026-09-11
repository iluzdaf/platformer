#include <algorithm>
#include <cmath>
#include <optional>
#include <vector>
#include "actor/behaviors/route_walker.hpp"
#include "actor/behaviors/footing.hpp"
#include "navigation/navigation_edge.hpp"
#include "actor/actor_behavior_context.hpp"
#include "input/input_intentions.hpp"
#include "navigation/navigation_graph.hpp"
#include "navigation/navigation_node.hpp"
#include "navigation/navigation_path.hpp"
#include "navigation/navigation_place.hpp"

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

    const NavigationEdge *edgeBetween(const NavigationGraph &navigationGraph, int fromId, int toId)
    {
        for (const auto &edge : navigationGraph.getOutgoingEdges(fromId))
            if (edge.toId == toId)
                return &edge;

        return nullptr;
    }
}

RouteWalker::RouteWalker(float arrivalThreshold) : arrivalThreshold(arrivalThreshold)
{
}

void RouteWalker::reset()
{
    currentNodeId.reset();
    targetNodeId.reset();
    stopShortAt.reset();
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

        float drop = node.feet.y - context.feet.y;
        if (drop < -SettlingTolerance)
            continue;

        float distance = std::abs(node.feet.x - context.feet.x);
        bool nearer = !currentNodeId || drop < nearestDrop ||
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
    if (!currentNodeId || !context.contacts.onGround)
        return false;

    const NavigationGraph &navigationGraph = context.navigationGraph;
    NavigationNode node = navigationGraph.getNode(*currentNodeId);
    if (!feetSettledOn(context.feet.y, node.feet.y))
        return true;

    float reach = context.colliderSize.x * 0.5f + arrivalThreshold;
    float leftEnd = node.feet.x;
    float rightEnd = node.feet.x;
    for (int id : walkableFrom(navigationGraph, *currentNodeId))
    {
        float x = navigationGraph.getNode(id).feet.x;
        leftEnd = std::min(leftEnd, x);
        rightEnd = std::max(rightEnd, x);
    }

    return context.feet.x < leftEnd - reach || context.feet.x > rightEnd + reach;
}

bool RouteWalker::walksGroundThatIsGone(const NavigationGraph &navigationGraph) const
{
    if (currentNodeId && !navigationGraph.hasNode(*currentNodeId))
        return true;

    if (targetNodeId && !navigationGraph.hasNode(*targetNodeId))
        return true;

    for (int id : legsLeft)
        if (!navigationGraph.hasNode(id))
            return true;

    return false;
}

void RouteWalker::keepInStep(const ActorBehaviorContext &context)
{
    if (walksGroundThatIsGone(context.navigationGraph) || hasLostTheRoute(context))
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
    if (!currentNodeId || !targetNodeId || !hasArrived(context, *currentNodeId, *targetNodeId))
        return;

    currentNodeId = targetNodeId;
    jumpHeldFor = 0.0f;

    legsLeft.erase(legsLeft.begin());
    targetNodeId = legsLeft.empty() ? std::nullopt : std::optional(legsLeft.front());
}

void RouteWalker::takeRouteTo(
    const ActorBehaviorContext &context,
    int destinationNodeId,
    std::optional<glm::vec2> stoppingShortAt)
{
    targetNodeId.reset();
    legsLeft.clear();
    stopShortAt = stoppingShortAt;

    if (!currentNodeId)
        return;

    if (destinationNodeId == *currentNodeId)
    {
        if (withinReachOf(context, destinationNodeId))
            return;

        legsLeft.assign(1, destinationNodeId);
        targetNodeId = destinationNodeId;
        return;
    }

    std::vector<int> route = findPath(context.navigationGraph, *currentNodeId, destinationNodeId);
    if (route.size() < 2)
        return;

    legsLeft.assign(route.begin() + 1, route.end());
    targetNodeId = legsLeft.front();
}

glm::vec2 RouteWalker::targetPosition(
    const ActorBehaviorContext &context,
    int setOffAt,
    int headingFor) const
{
    const NavigationGraph &navigationGraph = context.navigationGraph;
    glm::vec2 atTheNode = navigationGraph.getNode(headingFor).feet;

    const NavigationEdge *leg = edgeBetween(navigationGraph, setOffAt, headingFor);
    if (!stopShortAt || (leg && !travelledInContact(leg->type)))
        return atTheNode;

    glm::vec2 setOffFrom = navigationGraph.getNode(setOffAt).feet;
    glm::vec2 nearCorner = glm::min(setOffFrom, atTheNode) - glm::vec2(SurfaceTolerance);
    glm::vec2 farCorner = glm::max(setOffFrom, atTheNode) + glm::vec2(SurfaceTolerance);

    if (stopShortAt->x < nearCorner.x || stopShortAt->x > farCorner.x ||
        stopShortAt->y < nearCorner.y || stopShortAt->y > farCorner.y)
        return atTheNode;

    return *stopShortAt;
}

InputIntentions RouteWalker::follow(float deltaTime, const ActorBehaviorContext &context)
{
    InputIntentions inputIntentions;

    if (!currentNodeId || !targetNodeId)
        return inputIntentions;

    glm::vec2 target = targetPosition(context, *currentNodeId, *targetNodeId);
    inputIntentions.direction.x = directionTowards(context.feet.x, target.x);

    const NavigationEdge *leg = edgeBetween(context.navigationGraph, *currentNodeId, *targetNodeId);

    if (leg && leg->type == EdgeType::Jump && context.contacts.onGround &&
        jumpHeldFor >= leg->holdDuration)
        jumpHeldFor = 0.0f;

    if (leg && leg->type == EdgeType::Jump && jumpHeldFor == 0.0f)
    {
        NavigationNode takeOff = context.navigationGraph.getNode(*currentNodeId);
        if (std::abs(takeOff.feet.x - context.feet.x) > TakeOffReach)
        {
            inputIntentions.direction.x = directionTowards(context.feet.x, takeOff.feet.x);
            return inputIntentions;
        }
    }

    if (leg && leg->type == EdgeType::Fall && !context.contacts.onGround)
        inputIntentions.direction.x = 0.0f;

    if (leg && leg->type == EdgeType::Climb)
    {
        inputIntentions.climbRequested = true;
        if (context.contacts.touchingWall())
        {
            inputIntentions.direction.x = leg->wallDirection;
            inputIntentions.direction.y = directionTowards(context.feet.y, target.y);
        }
    }

    if (leg && leg->type == EdgeType::Jump && jumpHeldFor < leg->holdDuration)
    {
        jumpHeldFor += deltaTime;
        inputIntentions.jumpRequested = true;
        inputIntentions.jumpHeld = true;
    }

    return inputIntentions;
}

bool RouteWalker::withinReachOf(const ActorBehaviorContext &context, int nodeId) const
{
    NavigationNode node = context.navigationGraph.getNode(nodeId);
    if (!feetSettledOn(context.feet.y, node.feet.y))
        return false;

    float reach = context.colliderSize.x * 0.5f + arrivalThreshold;

    return std::abs(node.feet.x - context.feet.x) <= reach;
}

bool RouteWalker::hasArrived(const ActorBehaviorContext &context, int setOffAt, int headingFor)
    const
{
    const NavigationGraph &navigationGraph = context.navigationGraph;
    glm::vec2 target = targetPosition(context, setOffAt, headingFor);
    float reach = context.colliderSize.x * 0.5f + arrivalThreshold;

    const NavigationEdge *leg = edgeBetween(navigationGraph, setOffAt, headingFor);
    if (leg && leg->type == EdgeType::Climb)
    {
        if (std::abs(target.y - context.feet.y) <= SurfaceTolerance)
            return true;

        float climbDirection = directionTowards(navigationGraph.getNode(setOffAt).feet.y, target.y);

        return directionTowards(context.feet.y, target.y) != climbDirection;
    }

    if (!feetSettledOn(context.feet.y, target.y))
        return false;

    if (std::abs(target.x - context.feet.x) <= reach)
        return true;

    float legDirection = directionTowards(navigationGraph.getNode(setOffAt).feet.x, target.x);

    return directionTowards(context.feet.x, target.x) != legDirection;
}

std::optional<int> RouteWalker::getCurrentNodeId() const
{
    return currentNodeId;
}

std::optional<int> RouteWalker::getTargetNodeId() const
{
    return targetNodeId;
}
