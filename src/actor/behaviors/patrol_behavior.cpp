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
        std::optional<PlaceOnThePath> place = placeOnThePath(navigationGraph, asked);
        if (!place)
            return std::nullopt;

        glm::vec2 oneEnd = navigationGraph.getNode(place->fromId).position;
        glm::vec2 theOther = navigationGraph.getNode(place->toId).position;
        bool setOffTowardsTheFirst = glm::distance(context.worldPosition, oneEnd) >
                                     glm::distance(context.worldPosition, theOther);

        return BeatEnd{place->position, setOffTowardsTheFirst ? place->fromId : place->toId};
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

    walker.takeRouteTo(context, destination->routeVia, destination->position);
}
