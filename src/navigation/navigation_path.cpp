#include <cmath>
#include <algorithm>
#include <optional>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <glm/geometric.hpp>
#include "navigation/navigation_path.hpp"
#include "navigation/navigation_edge.hpp"
#include "navigation/navigation_graph.hpp"

namespace
{
    constexpr float SurfaceTolerance = 1.0f;

    struct Step
    {
        float estimate = 0.0f;
        int id = 0;
    };

    bool furtherThan(const Step &step, const Step &against)
    {
        return step.estimate > against.estimate;
    }

    using Ways = std::unordered_map<int, std::vector<int>>;

    Ways waysOn(const NavigationGraph &navigationGraph, bool onFoot)
    {
        Ways ways;
        for (const NavigationEdge &edge : navigationGraph.getEdges())
            if (!onFoot || edge.type == EdgeType::Walk)
                ways[edge.fromId].push_back(edge.toId);

        return ways;
    }

    Ways waysBack(const NavigationGraph &navigationGraph)
    {
        Ways ways;
        for (const NavigationEdge &edge : navigationGraph.getEdges())
            ways[edge.toId].push_back(edge.fromId);

        return ways;
    }

    std::unordered_set<int> spreadFrom(int fromId, const Ways &ways)
    {
        std::unordered_set<int> found{fromId};

        std::vector<int> pending{fromId};
        while (!pending.empty())
        {
            int at = pending.back();
            pending.pop_back();

            auto onward = ways.find(at);
            if (onward == ways.end())
                continue;

            for (int toId : onward->second)
                if (found.insert(toId).second)
                    pending.push_back(toId);
        }

        return found;
    }

    std::vector<int> inIdOrder(const std::unordered_set<int> &ids)
    {
        std::vector<int> ordered(ids.begin(), ids.end());
        std::ranges::sort(ordered);
        return ordered;
    }

    std::vector<int> retrace(const std::unordered_map<int, int> &arrivedFrom, int fromId, int toId)
    {
        std::vector<int> route{toId};
        for (int at = toId; at != fromId;)
        {
            at = arrivedFrom.at(at);
            route.push_back(at);
        }

        std::ranges::reverse(route);
        return route;
    }
}

std::vector<int> findPath(const NavigationGraph &navigationGraph, int fromId, int toId)
{
    glm::vec2 start = navigationGraph.getNode(fromId).position;
    glm::vec2 goal = navigationGraph.getNode(toId).position;

    if (fromId == toId)
        return {fromId};

    std::priority_queue<Step, std::vector<Step>, decltype(&furtherThan)> pending(&furtherThan);
    std::unordered_map<int, float> travelled{{fromId, 0.0f}};
    std::unordered_map<int, int> arrivedFrom;
    std::unordered_set<int> settled;

    pending.push({glm::distance(start, goal), fromId});

    while (!pending.empty())
    {
        int at = pending.top().id;
        pending.pop();

        if (at == toId)
            return retrace(arrivedFrom, fromId, toId);

        if (!settled.insert(at).second)
            continue;

        glm::vec2 here = navigationGraph.getNode(at).position;
        for (const NavigationEdge &edge : navigationGraph.getOutgoingEdges(at))
        {
            glm::vec2 there = navigationGraph.getNode(edge.toId).position;
            float cost = travelled.at(at) + glm::distance(here, there);

            auto found = travelled.find(edge.toId);
            if (found != travelled.end() && found->second <= cost)
                continue;

            travelled[edge.toId] = cost;
            arrivedFrom[edge.toId] = at;
            pending.push({cost + glm::distance(there, goal), edge.toId});
        }
    }

    return {};
}

std::vector<int> roundTripFrom(const NavigationGraph &navigationGraph, int fromId)
{
    navigationGraph.getNode(fromId);

    std::unordered_set<int> out = spreadFrom(fromId, waysOn(navigationGraph, false));
    std::unordered_set<int> back = spreadFrom(fromId, waysBack(navigationGraph));

    std::unordered_set<int> both;
    for (int id : out)
        if (back.contains(id))
            both.insert(id);

    return inIdOrder(both);
}

std::vector<int> walkableFrom(const NavigationGraph &navigationGraph, int fromId)
{
    navigationGraph.getNode(fromId);

    return inIdOrder(spreadFrom(fromId, waysOn(navigationGraph, true)));
}

std::optional<int> nearestNodeTo(const NavigationGraph &navigationGraph, glm::vec2 position)
{
    std::optional<int> nearest;
    float nearestDistance = 0.0f;
    for (const auto &[id, node] : navigationGraph.getNodes())
    {
        float distance = glm::distance(node.position, position);
        if (nearest && distance >= nearestDistance)
            continue;

        nearest = id;
        nearestDistance = distance;
    }

    return nearest;
}

namespace
{
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
