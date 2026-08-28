#include <optional>
#include <glm/geometric.hpp>
#include <utility>
#include "actor/behaviors/patrol_behavior.hpp"
#include "actor/behaviors/patrol_behavior_data.hpp"
#include "actor/actor_behavior_context.hpp"
#include "input/input_intentions.hpp"
#include "navigation/navigation_graph.hpp"
#include "navigation/navigation_path.hpp"

PatrolBehavior::PatrolBehavior(
    const PatrolBehaviorData &data,
    std::optional<std::pair<glm::vec2, glm::vec2>> patrolBetween)
    : data(data), walker(data.arrivalThreshold), patrolBetween(patrolBetween)
{
}

std::optional<int> PatrolBehavior::endOfTheBeat(const ActorBehaviorContext &context, bool second)
    const
{
    const NavigationGraph &navigationGraph = context.navigationGraph;

    if (patrolBetween)
    {
        return nearestNodeTo(
            navigationGraph, second ? patrolBetween->second : patrolBetween->first);
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

    return end;
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

    std::optional<int> destination = endOfTheBeat(context, headingForTheSecond);

    if (destination && *destination == *from)
    {
        headingForTheSecond = !headingForTheSecond;
        destination = endOfTheBeat(context, headingForTheSecond);
    }

    if (!destination)
        return;

    walker.takeRouteTo(context, *destination);
}
