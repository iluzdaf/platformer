#include <optional>
#include <string_view>
#include "actor/actor_behavior.hpp"

std::string_view ActorBehavior::getStateName() const
{
    return {};
}

void ActorBehavior::reset()
{
}

void ActorBehavior::leave()
{
}

void ActorBehavior::scriptWith(StateScript *)
{
}

std::optional<int> ActorBehavior::getCurrentNodeId() const
{
    return std::nullopt;
}

std::optional<int> ActorBehavior::getTargetNodeId() const
{
    return std::nullopt;
}
