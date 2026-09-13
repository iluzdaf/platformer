#include <optional>
#include "scripting/script_walker.hpp"
#include "actor/actor_facts.hpp"
#include "actor/behaviors/route_walker.hpp"
#include "input/input_intentions.hpp"

ScriptWalker::ScriptWalker(RouteWalker &walker, const ActorFacts &facts)
    : walker(walker), facts(facts)
{
}

bool ScriptWalker::anchored() const
{
    return walker.isAnchored();
}

bool ScriptWalker::finished() const
{
    return walker.routeFinished();
}

void ScriptWalker::routeTo(int node)
{
    walker.takeRouteTo(facts, node);
}

InputIntentions ScriptWalker::follow(float deltaTime)
{
    return walker.follow(deltaTime, facts);
}

std::optional<int> ScriptWalker::currentNode() const
{
    return walker.getCurrentNodeId();
}

std::optional<int> ScriptWalker::targetNode() const
{
    return walker.getTargetNodeId();
}
