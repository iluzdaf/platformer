#include <cmath>
#include <optional>
#include <glm/geometric.hpp>
#include <utility>
#include "actor/behaviors/patrol_behavior.hpp"
#include "actor/behaviors/patrol_behavior_data.hpp"
#include "actor/actor_behavior_context.hpp"
#include "input/input_intentions.hpp"
#include "navigation/navigation_graph.hpp"
#include "navigation/navigation_path.hpp"
#include "navigation/navigation_node.hpp"

namespace
{
    constexpr float SurfaceTolerance = 1.0f;
}

PatrolBehavior::PatrolBehavior(
    const PatrolBehaviorData &data,
    std::optional<std::pair<glm::vec2, glm::vec2>> patrolBetween)
    : data(data), walker(data.arrivalThreshold), patrolBetween(patrolBetween)
{
}

std::optional<PatrolBehavior::BeatEnd> PatrolBehavior::endOfTheBeat(
    const ActorBehaviorContext &context,
    bool second) const
{
    const NavigationGraph &navigationGraph = context.navigationGraph;

    if (patrolBetween)
    {
        glm::vec2 asked = second ? patrolBetween->second : patrolBetween->first;
        std::optional<int> onTheRun = nodeUnderfoot(navigationGraph, asked);
        if (!onTheRun)
            return std::nullopt;

        return alongTheRunFrom(context, *onTheRun, asked.x);
    }

    std::optional<int> from = walker.getCurrentNodeId();
    if (!from)
        return std::nullopt;

    std::optional<int> end;
    for (int id : walkableFrom(navigationGraph, *from))
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

    if (!end)
        return std::nullopt;

    return BeatEnd{navigationGraph.getNode(*end).position, *end};
}

PatrolBehavior::BeatEnd PatrolBehavior::alongTheRunFrom(
    const ActorBehaviorContext &context,
    int onTheRun,
    float askedX) const
{
    const NavigationGraph &navigationGraph = context.navigationGraph;
    float surface = navigationGraph.getNode(onTheRun).position.y;

    int leftmost = onTheRun, rightmost = onTheRun;
    for (int id : walkableFrom(navigationGraph, onTheRun))
    {
        NavigationNode node = navigationGraph.getNode(id);
        if (std::abs(node.position.y - surface) > SurfaceTolerance)
            continue;

        if (node.position.x < navigationGraph.getNode(leftmost).position.x)
            leftmost = id;

        if (node.position.x > navigationGraph.getNode(rightmost).position.x)
            rightmost = id;
    }

    float stopAt = placeOnTheRun(navigationGraph, glm::vec2(askedX, surface)).x;

    bool headingRight = context.worldPosition.x <= stopAt;
    int routeVia = headingRight ? rightmost : leftmost;
    for (int id : walkableFrom(navigationGraph, onTheRun))
    {
        NavigationNode node = navigationGraph.getNode(id);
        if (std::abs(node.position.y - surface) > SurfaceTolerance)
            continue;

        float here = node.position.x;
        float best = navigationGraph.getNode(routeVia).position.x;
        if (headingRight ? (here >= stopAt && here < best) : (here <= stopAt && here > best))
            routeVia = id;
    }

    return BeatEnd{glm::vec2(stopAt, surface), routeVia};
}

bool PatrolBehavior::standingAt(const ActorBehaviorContext &context, const BeatEnd &end) const
{
    if (std::abs(context.worldPosition.y - end.position.y) > SurfaceTolerance)
        return false;

    float reach = context.colliderSize.x * 0.5f + data.arrivalThreshold;

    return std::abs(context.worldPosition.x - end.position.x) <= reach;
}

void PatrolBehavior::reset()
{
    walker.reset();
    headingForTheSecond = false;
}

InputIntentions PatrolBehavior::decide(float deltaTime, const ActorBehaviorContext &context)
{
    walker.keepInStep(context);
    if (!walker.isAnchored())
        return InputIntentions();

    walker.advanceOnArrival(context);
    if (walker.routeFinished())
        planRoute(context);

    return walker.follow(deltaTime, context);
}

std::optional<int> PatrolBehavior::getCurrentNodeId() const
{
    return walker.getCurrentNodeId();
}

std::optional<int> PatrolBehavior::getTargetNodeId() const
{
    return walker.getTargetNodeId();
}

void PatrolBehavior::planRoute(const ActorBehaviorContext &context)
{
    std::optional<int> from = walker.getCurrentNodeId();
    if (!from)
        return;

    std::optional<BeatEnd> destination = endOfTheBeat(context, headingForTheSecond);

    if (destination && standingAt(context, *destination))
    {
        headingForTheSecond = !headingForTheSecond;
        destination = endOfTheBeat(context, headingForTheSecond);
    }

    if (!destination)
        return;

    walker.takeRouteTo(context, destination->routeVia, destination->position.x);
}
