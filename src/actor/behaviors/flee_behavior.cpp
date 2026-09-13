#include <glm/geometric.hpp>
#include <optional>
#include "actor/behaviors/flee_behavior.hpp"
#include "actor/behaviors/flee_behavior_data.hpp"
#include "actor/actor_facts.hpp"
#include "input/input_intentions.hpp"
#include "navigation/navigation_graph.hpp"
#include "navigation/navigation_path.hpp"
#include "navigation/navigation_place.hpp"

FleeBehavior::FleeBehavior(const FleeBehaviorData &data) : data(data), walker(data.arrivalThreshold)
{
}

void FleeBehavior::reset()
{
    walker.reset();
}

std::optional<int> FleeBehavior::furthestFrom(const ActorFacts &context) const
{
    std::optional<int> from = walker.getCurrentNodeId();
    if (!from || !context.threatFeet)
        return std::nullopt;

    float away = context.feet.x < context.threatFeet->x ? -1.0f : 1.0f;
    return furthestRefugeFrom(context.navigationGraph, *from, *context.threatFeet, away);
}

bool FleeBehavior::fleeingTowardsTheThreat(const ActorFacts &context) const
{
    std::optional<int> destination = walker.getTargetNodeId();
    if (!destination || !context.threatFeet || !context.contacts.onGround)
        return false;

    glm::vec2 refuge = context.navigationGraph.getNode(*destination).feet;

    return glm::distance(refuge, *context.threatFeet) <
           glm::distance(context.feet, *context.threatFeet);
}

void FleeBehavior::planRoute(const ActorFacts &context)
{
    std::optional<int> refuge = furthestFrom(context);
    if (!refuge)
        return;

    walker.takeRouteTo(context, *refuge);
}

InputIntentions FleeBehavior::decide(float deltaTime, const ActorFacts &context)
{
    walker.keepInStep(context);
    if (!walker.isAnchored())
        return InputIntentions();

    walker.advanceOnArrival(context);
    if (walker.routeFinished() || fleeingTowardsTheThreat(context))
        planRoute(context);

    return walker.follow(deltaTime, context);
}

std::optional<int> FleeBehavior::getCurrentNodeId() const
{
    return walker.getCurrentNodeId();
}

std::optional<int> FleeBehavior::getTargetNodeId() const
{
    return walker.getTargetNodeId();
}
