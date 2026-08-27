#include <algorithm>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <glm/geometric.hpp>
#include "navigation/navigation_path.hpp"
#include "navigation/navigation_graph.hpp"

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

    // A*, where an edge costs the distance it covers and the guess at what is
    // left is the straight line to the goal, which no route can beat.
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
