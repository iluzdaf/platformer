#include <cmath>
#include <algorithm>
#include <optional>
#include <vector>
#include <glm/geometric.hpp>
#include "navigation/navigation_place.hpp"
#include "navigation/navigation_path.hpp"
#include "navigation/navigation_edge.hpp"
#include "navigation/navigation_graph.hpp"

namespace
{
    constexpr float SurfaceTolerance = 1.0f;

    glm::vec2 nearestPointOn(glm::vec2 from, glm::vec2 to, glm::vec2 asked)
    {
        glm::vec2 along = to - from;
        float lengthSquared = glm::dot(along, along);
        if (lengthSquared == 0.0f)
            return from;

        float howFar = std::clamp(glm::dot(asked - from, along) / lengthSquared, 0.0f, 1.0f);

        return from + along * howFar;
    }

    std::optional<int> nodeUnderfoot(const NavigationGraph &navigationGraph, glm::vec2 position)
    {
        std::optional<int> standingOn;
        float nearestAlong = 0.0f;
        for (const auto &[id, node] : navigationGraph.getNodes())
        {
            if (std::abs(node.position.y - position.y) > SurfaceTolerance)
                continue;

            float along = std::abs(node.position.x - position.x);
            if (standingOn && along >= nearestAlong)
                continue;

            standingOn = id;
            nearestAlong = along;
        }

        return standingOn ? standingOn : nearestNodeTo(navigationGraph, position);
    }
}

bool travelledInContact(EdgeType type)
{
    return type == EdgeType::Walk || type == EdgeType::Climb;
}

std::optional<PlaceOnThePath> placeOnThePath(
    const NavigationGraph &navigationGraph,
    glm::vec2 asked)
{
    std::optional<PlaceOnThePath> nearest;
    float nearestDistance = 0.0f;

    auto consider = [&](PlaceOnThePath place)
    {
        float distance = glm::distance(place.position, asked);
        if (nearest && distance >= nearestDistance)
            return;

        nearest = place;
        nearestDistance = distance;
    };

    for (const auto &[id, node] : navigationGraph.getNodes())
        consider(PlaceOnThePath{node.position, id, id});

    for (const auto &[id, node] : navigationGraph.getNodes())
        for (const NavigationEdge &edge : navigationGraph.getOutgoingEdges(id))
        {
            if (!travelledInContact(edge.type))
                continue;

            consider(
                PlaceOnThePath{
                    nearestPointOn(
                        navigationGraph.getNode(edge.fromId).position,
                        navigationGraph.getNode(edge.toId).position,
                        asked),
                    edge.fromId,
                    edge.toId});
        }

    return nearest;
}

int endOfThePathTowards(
    const NavigationGraph &navigationGraph,
    const PlaceOnThePath &place,
    glm::vec2 towards)
{
    glm::vec2 oneEnd = navigationGraph.getNode(place.fromId).position;
    glm::vec2 theOther = navigationGraph.getNode(place.toId).position;

    return glm::distance(towards, oneEnd) <= glm::distance(towards, theOther) ? place.fromId
                                                                              : place.toId;
}

int endOfThePathBeyond(
    const NavigationGraph &navigationGraph,
    const PlaceOnThePath &place,
    glm::vec2 comingFrom)
{
    glm::vec2 travelling = place.position - comingFrom;
    glm::vec2 oneEnd = navigationGraph.getNode(place.fromId).position;

    return glm::dot(oneEnd - place.position, travelling) >= 0.0f ? place.fromId : place.toId;
}

bool onTheSameRun(const NavigationGraph &navigationGraph, glm::vec2 here, glm::vec2 there)
{
    std::optional<int> from = nodeUnderfoot(navigationGraph, here);
    std::optional<int> to = nodeUnderfoot(navigationGraph, there);
    if (!from || !to)
        return false;

    if (*from == *to)
        return true;

    std::vector<int> run = walkableFrom(navigationGraph, *from);

    return std::find(run.begin(), run.end(), *to) != run.end();
}

bool canPatrolBetween(const NavigationGraph &navigationGraph, glm::vec2 from, glm::vec2 to)
{
    std::optional<PlaceOnThePath> setsOff = placeOnThePath(navigationGraph, from);
    std::optional<PlaceOnThePath> turnsRound = placeOnThePath(navigationGraph, to);
    if (!setsOff || !turnsRound)
        return false;

    if (setsOff->fromId == turnsRound->fromId)
        return true;

    std::vector<int> andBack = roundTripFrom(navigationGraph, setsOff->fromId);

    return std::find(andBack.begin(), andBack.end(), turnsRound->fromId) != andBack.end();
}
