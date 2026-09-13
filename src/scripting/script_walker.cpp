#include <optional>
#include "scripting/script_walker.hpp"
#include "actor/actor_facts.hpp"
#include "actor/behaviors/route_walker.hpp"
#include "input/input_intentions.hpp"
#include "navigation/navigation_graph.hpp"
#include "navigation/navigation_node.hpp"
#include "navigation/navigation_place.hpp"
#include "navigation/navigation_path.hpp"
#include <vector>
#include <glm/gtc/matrix_transform.hpp>

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

void ScriptWalker::routeTo(int node, std::optional<glm::vec2> stopShortAt)
{
    walker.takeRouteTo(facts, node, stopShortAt);
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

glm::vec2 ScriptWalker::feetOf(int node) const
{
    return facts.navigationGraph.getNode(node).feet;
}

std::optional<int> ScriptWalker::furthestRefugeFrom(int from, glm::vec2 threat, float away) const
{
    return ::furthestRefugeFrom(facts.navigationGraph, from, threat, away);
}

std::optional<PlaceOnThePath> ScriptWalker::placeOnThePath(glm::vec2 point) const
{
    return ::placeOnThePath(facts.navigationGraph, point);
}

int ScriptWalker::endOfThePathBeyond(const PlaceOnThePath &place, glm::vec2 comingFrom) const
{
    return ::endOfThePathBeyond(facts.navigationGraph, place, comingFrom);
}

std::vector<int> ScriptWalker::walkableFrom(int node) const
{
    return ::walkableFrom(facts.navigationGraph, node);
}

bool ScriptWalker::standsAt(glm::vec2 point) const
{
    return walker.standsAt(facts, point);
}
