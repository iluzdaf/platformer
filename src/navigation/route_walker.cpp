#include <algorithm>
#include <cmath>
#include <optional>
#include <vector>
#include "navigation/route_walker.hpp"
#include "navigation/footing.hpp"
#include "navigation/input_program.hpp"
#include "navigation/navigation_edge.hpp"
#include "actor/actor_facts.hpp"
#include "input/input_intentions.hpp"
#include "navigation/navigation_graph.hpp"
#include "navigation/navigation_node.hpp"
#include "navigation/navigation_path.hpp"
#include "navigation/navigation_place.hpp"

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
    replayedFor = 0.0f;
    leftTheGround = false;
}

void RouteWalker::anchor(const ActorFacts &context)
{
    float nearestDrop = 0.0f;
    float nearestDistance = 0.0f;
    for (const auto &[id, node] : context.navigationGraph.getNodes())
    {
        if (!hasSomewhereToGo(context.navigationGraph, id))
            continue;

        float drop = feetSettledOn(context.feet.y, node.feet.y, context.stepHeight)
                         ? 0.0f
                         : node.feet.y - context.feet.y;
        if (drop < 0.0f)
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

bool RouteWalker::hasLostTheRoute(const ActorFacts &context) const
{
    if (!currentNodeId || !context.contacts.onGround)
        return false;

    const NavigationGraph &navigationGraph = context.navigationGraph;
    glm::vec2 node = navigationGraph.getNode(*currentNodeId).feet;
    glm::vec2 nearCorner = node;
    glm::vec2 farCorner = node;
    for (int id : walkableFrom(navigationGraph, *currentNodeId))
    {
        nearCorner = glm::min(nearCorner, navigationGraph.getNode(id).feet);
        farCorner = glm::max(farCorner, navigationGraph.getNode(id).feet);
    }

    float reach = context.colliderSize.x * 0.5f + arrivalThreshold;
    float settle = settlingTolerance(context.stepHeight);

    return context.feet.x < nearCorner.x - reach || context.feet.x > farCorner.x + reach ||
           context.feet.y < nearCorner.y - settle || context.feet.y > farCorner.y + settle;
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

void RouteWalker::keepInStep(const ActorFacts &context)
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

void RouteWalker::advanceOnArrival(const ActorFacts &context)
{
    if (!currentNodeId || !targetNodeId || !hasArrived(context, *currentNodeId, *targetNodeId))
        return;

    currentNodeId = targetNodeId;
    replayedFor = 0.0f;
    leftTheGround = false;

    legsLeft.erase(legsLeft.begin());
    targetNodeId = legsLeft.empty() ? std::nullopt : std::optional(legsLeft.front());
}

void RouteWalker::takeRouteTo(
    const ActorFacts &context,
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

glm::vec2 RouteWalker::targetPosition(const ActorFacts &context, int setOffAt, int headingFor) const
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

InputIntentions RouteWalker::follow(float deltaTime, const ActorFacts &context)
{
    InputIntentions inputIntentions;

    if (!currentNodeId || !targetNodeId)
        return inputIntentions;

    glm::vec2 target = targetPosition(context, *currentNodeId, *targetNodeId);
    inputIntentions.direction.x = directionTowards(context.feet.x, target.x);

    const NavigationEdge *leg = edgeBetween(context.navigationGraph, *currentNodeId, *targetNodeId);

    bool replayed = leg && (leg->type == EdgeType::Jump || leg->type == EdgeType::Fall);
    NavigationNode takeOff = context.navigationGraph.getNode(*currentNodeId);
    bool atTheTakeOff = std::abs(takeOff.feet.x - context.feet.x) <= TakeOffReach;

    if (replayed && !context.contacts.onGround)
        leftTheGround = true;

    if (replayed && context.contacts.onGround && replayedFor >= durationOf(leg->inputs) &&
        (leftTheGround || atTheTakeOff))
    {
        replayedFor = 0.0f;
        leftTheGround = false;
    }

    if (replayed && replayedFor == 0.0f && !(atTheTakeOff && context.contacts.onGround))
    {
        inputIntentions.direction.x = directionTowards(context.feet.x, takeOff.feet.x);
        return inputIntentions;
    }

    if (leg && leg->type == EdgeType::Climb)
    {
        inputIntentions.climbRequested = true;
        inputIntentions.direction.y = directionTowards(context.feet.y, target.y);
        if (context.contacts.touchingWall())
            inputIntentions.direction.x = leg->wallDirection;
    }

    if (replayed)
    {
        inputIntentions = replaying(
            leg->inputs, replayedFor, context.feet.x, target.x, context.moveSpeed * deltaTime);
        replayedFor += deltaTime;
    }

    return inputIntentions;
}

bool RouteWalker::withinReachOf(const ActorFacts &context, int nodeId) const
{
    return standsAt(context, context.navigationGraph.getNode(nodeId).feet);
}

bool RouteWalker::standsAt(const ActorFacts &context, glm::vec2 point, float atLeast) const
{
    if (!feetSettledOn(context.feet.y, point.y, context.stepHeight))
        return false;

    float reach = std::max(context.colliderSize.x * 0.5f + arrivalThreshold, atLeast);

    return std::abs(point.x - context.feet.x) <= reach;
}

float RouteWalker::arrivesWithin() const
{
    return arrivalThreshold;
}

bool RouteWalker::hasArrived(const ActorFacts &context, int setOffAt, int headingFor) const
{
    const NavigationGraph &navigationGraph = context.navigationGraph;
    glm::vec2 target = targetPosition(context, setOffAt, headingFor);
    float reach = context.colliderSize.x * 0.5f + arrivalThreshold;

    const NavigationEdge *leg = edgeBetween(navigationGraph, setOffAt, headingFor);
    if (leg && leg->type == EdgeType::Climb)
    {
        if (std::abs(target.y - context.feet.y) <= ClimbArrivesWithin)
            return true;

        float climbDirection = directionTowards(navigationGraph.getNode(setOffAt).feet.y, target.y);

        return directionTowards(context.feet.y, target.y) != climbDirection;
    }

    if (!feetSettledOn(context.feet.y, target.y, context.stepHeight))
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
