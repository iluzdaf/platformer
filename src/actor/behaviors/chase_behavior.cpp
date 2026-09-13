#include <algorithm>
#include <cmath>
#include <glm/geometric.hpp>
#include <optional>
#include <vector>
#include "actor/behaviors/chase_behavior.hpp"
#include "actor/behaviors/footing.hpp"
#include "actor/behaviors/chase_behavior_data.hpp"
#include "actor/actor_facts.hpp"
#include "input/input_intentions.hpp"
#include "navigation/navigation_graph.hpp"
#include "navigation/navigation_node.hpp"
#include "navigation/navigation_path.hpp"
#include "navigation/navigation_place.hpp"

ChaseBehavior::ChaseBehavior(const ChaseBehaviorData &data)
    : data(data), walker(data.arrivalThreshold)
{
}

void ChaseBehavior::reset()
{
    walker.reset();
    lastSeenAt.reset();
}

bool ChaseBehavior::caughtUp(const ActorFacts &context) const
{
    if (!context.threatFeet)
        return false;

    if (!feetSettledOn(context.feet.y, context.threatFeet->y))
        return false;

    float reach = context.colliderSize.x * 0.5f + data.arrivalThreshold;

    return std::abs(context.threatFeet->x - context.feet.x) <= std::max(reach, data.standoff);
}

bool ChaseBehavior::threatHasMoved(const ActorFacts &context) const
{
    if (!lastSeenAt || !context.threatFeet)
        return true;

    return glm::distance(*context.threatFeet, *lastSeenAt) > data.arrivalThreshold;
}

std::optional<int> ChaseBehavior::whereToCloseIn(const ActorFacts &context) const
{
    std::optional<int> from = walker.getCurrentNodeId();
    if (!from || !context.threatFeet)
        return std::nullopt;

    const NavigationGraph &navigationGraph = context.navigationGraph;
    std::vector<int> reachable = roundTripFrom(navigationGraph, *from);

    if (std::optional<PlaceOnThePath> place = placeOnThePath(navigationGraph, *context.threatFeet))
    {
        int beyond = endOfThePathBeyond(navigationGraph, *place, context.feet);
        if (std::ranges::find(reachable, beyond) != reachable.end())
            return beyond;
    }

    std::optional<int> nearest;
    float nearestDistance = 0.0f;
    for (int id : reachable)
    {
        float distance = glm::distance(navigationGraph.getNode(id).feet, *context.threatFeet);
        if (nearest && distance >= nearestDistance)
            continue;

        nearest = id;
        nearestDistance = distance;
    }

    return nearest;
}

void ChaseBehavior::planRoute(const ActorFacts &context)
{
    lastSeenAt = context.threatFeet;
    std::optional<int> quarry = whereToCloseIn(context);
    if (!quarry)
        return;

    walker.takeRouteTo(context, *quarry, context.threatFeet);
}

InputIntentions ChaseBehavior::decide(float deltaTime, const ActorFacts &context)
{
    walker.keepInStep(context);
    if (!walker.isAnchored())
        return InputIntentions();

    if (!context.threatFeet)
        return InputIntentions();

    if (caughtUp(context))
        return InputIntentions();

    walker.advanceOnArrival(context);
    if (walker.routeFinished() || (context.contacts.onGround && threatHasMoved(context)))
        planRoute(context);

    return walker.follow(deltaTime, context);
}

std::optional<int> ChaseBehavior::getCurrentNodeId() const
{
    return walker.getCurrentNodeId();
}

std::optional<int> ChaseBehavior::getTargetNodeId() const
{
    return walker.getTargetNodeId();
}
