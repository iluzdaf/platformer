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
#include "navigation/navigation_place.hpp"

namespace
{
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

    float quickestPace(const NavigationGraph &navigationGraph)
    {
        std::optional<float> quickest;
        for (const NavigationEdge &edge : navigationGraph.getEdges())
        {
            float across = glm::distance(
                navigationGraph.getNode(edge.fromId).feet, navigationGraph.getNode(edge.toId).feet);
            if (across <= 0.0f)
                continue;

            float pace = costOf(navigationGraph, edge) / across;
            if (!quickest || pace < *quickest)
                quickest = pace;
        }

        return quickest.value_or(0.0f);
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

float costOf(const NavigationGraph &navigationGraph, const NavigationEdge &edge)
{
    if (edge.duration)
        return *edge.duration;

    return glm::distance(
        navigationGraph.getNode(edge.fromId).feet, navigationGraph.getNode(edge.toId).feet);
}

std::vector<int> findPath(const NavigationGraph &navigationGraph, int fromId, int toId)
{
    glm::vec2 start = navigationGraph.getNode(fromId).feet;
    glm::vec2 goal = navigationGraph.getNode(toId).feet;

    if (fromId == toId)
        return {fromId};

    float pace = quickestPace(navigationGraph);
    std::priority_queue<Step, std::vector<Step>, decltype(&furtherThan)> pending(&furtherThan);
    std::unordered_map<int, float> travelled{{fromId, 0.0f}};
    std::unordered_map<int, int> arrivedFrom;
    std::unordered_set<int> settled;

    pending.push({glm::distance(start, goal) * pace, fromId});

    while (!pending.empty())
    {
        int at = pending.top().id;
        pending.pop();

        if (at == toId)
            return retrace(arrivedFrom, fromId, toId);

        if (!settled.insert(at).second)
            continue;

        for (const NavigationEdge &edge : navigationGraph.getOutgoingEdges(at))
        {
            glm::vec2 there = navigationGraph.getNode(edge.toId).feet;
            float cost = travelled.at(at) + costOf(navigationGraph, edge);

            auto found = travelled.find(edge.toId);
            if (found != travelled.end() && found->second <= cost)
                continue;

            travelled[edge.toId] = cost;
            arrivedFrom[edge.toId] = at;
            pending.push({cost + glm::distance(there, goal) * pace, edge.toId});
        }
    }

    return {};
}

std::unordered_map<int, float> costsFrom(const NavigationGraph &navigationGraph, int fromId)
{
    navigationGraph.getNode(fromId);

    std::priority_queue<Step, std::vector<Step>, decltype(&furtherThan)> pending(&furtherThan);
    std::unordered_map<int, float> costs{{fromId, 0.0f}};
    std::unordered_set<int> settled;
    pending.push({0.0f, fromId});

    while (!pending.empty())
    {
        int at = pending.top().id;
        pending.pop();
        if (!settled.insert(at).second)
            continue;

        for (const NavigationEdge &edge : navigationGraph.getOutgoingEdges(at))
        {
            float cost = costs.at(at) + costOf(navigationGraph, edge);
            auto found = costs.find(edge.toId);
            if (found != costs.end() && found->second <= cost)
                continue;

            costs[edge.toId] = cost;
            pending.push({cost, edge.toId});
        }
    }

    return costs;
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

bool connectedInContact(const NavigationGraph &navigationGraph, int fromId, int toId)
{
    navigationGraph.getNode(fromId);
    navigationGraph.getNode(toId);

    Ways ways;
    for (const NavigationEdge &edge : navigationGraph.getEdges())
        if (travelledInContact(edge.type))
        {
            ways[edge.fromId].push_back(edge.toId);
            ways[edge.toId].push_back(edge.fromId);
        }

    return spreadFrom(fromId, ways).contains(toId);
}

std::optional<int> nearestNodeTo(const NavigationGraph &navigationGraph, glm::vec2 position)
{
    std::optional<int> nearest;
    float nearestDistance = 0.0f;
    for (const auto &[id, node] : navigationGraph.getNodes())
    {
        float distance = glm::distance(node.feet, position);
        if (nearest && distance >= nearestDistance)
            continue;

        nearest = id;
        nearestDistance = distance;
    }

    return nearest;
}
