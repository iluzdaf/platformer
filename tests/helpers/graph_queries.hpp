#pragma once

#include <algorithm>
#include <cmath>
#include <set>
#include <utility>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "navigation/navigation_edge.hpp"
#include "navigation/navigation_graph.hpp"
#include "navigation/navigation_node.hpp"
#include "navigation/navigation_path.hpp"
#include <optional>
#include "tile_map/tile_map.hpp"

inline std::vector<float> nodeXsOnRow(const NavigationGraph &graph, float y)
{
    std::vector<float> xs;
    for (const auto &[id, node] : graph.getNodes())
        if (node.feet.y == y)
            xs.push_back(node.feet.x);
    std::sort(xs.begin(), xs.end());
    return xs;
}

inline bool hasEdgeBetween(const NavigationGraph &graph, float fromX, float toX, float y)
{
    for (const auto &edge : graph.getEdges())
    {
        NavigationNode from = graph.getNode(edge.fromId);
        NavigationNode to = graph.getNode(edge.toId);
        if (from.feet == glm::vec2(fromX, y) && to.feet == glm::vec2(toX, y))
            return true;
    }
    return false;
}

inline bool hasEdgeBetween(
    const NavigationGraph &graph,
    glm::vec2 from,
    glm::vec2 to,
    EdgeType type)
{
    constexpr float Tolerance = 0.5f;
    for (const auto &edge : graph.getEdges())
    {
        if (edge.type != type)
            continue;

        if (glm::distance(graph.getNode(edge.fromId).feet, from) > Tolerance)
            continue;

        if (glm::distance(graph.getNode(edge.toId).feet, to) <= Tolerance)
            return true;
    }

    return false;
}

inline bool isReachable(const NavigationGraph &graph, glm::vec2 from, glm::vec2 to)
{
    std::optional<int> fromId = graph.nodeAtPosition(from);
    std::optional<int> toId = graph.nodeAtPosition(to);
    return fromId && toId && !findPath(graph, *fromId, *toId).empty();
}

inline int countEdgesOfType(const NavigationGraph &graph, EdgeType type)
{
    int count = 0;
    for (const auto &edge : graph.getEdges())
        if (edge.type == type)
            ++count;
    return count;
}

inline bool jumpsAcrossTo(const NavigationGraph &graph, glm::vec2 from, glm::vec2 farSide)
{
    constexpr float Tolerance = 0.5f;
    for (const auto &edge : graph.getEdges())
    {
        if (edge.type != EdgeType::Jump)
            continue;

        if (glm::distance(graph.getNode(edge.fromId).feet, from) > Tolerance)
            continue;

        NavigationNode to = graph.getNode(edge.toId);
        if (std::abs(to.feet.y - farSide.y) > Tolerance)
            continue;

        bool overThere = farSide.x > from.x ? to.feet.x >= farSide.x - Tolerance
                                            : to.feet.x <= farSide.x + Tolerance;
        if (overThere)
            return true;
    }

    return false;
}

inline const NavigationEdge *onlyJumpFrom(const NavigationGraph &graph, int fromId)
{
    const NavigationEdge *only = nullptr;
    for (const auto &edge : graph.getOutgoingEdges(fromId))
        if (edge.type == EdgeType::Jump)
        {
            if (only)
                return nullptr;
            only = &edge;
        }

    return only;
}

inline std::vector<NavigationNode> nodesOnWalls(const NavigationGraph &graph)
{
    std::vector<NavigationNode> onWalls;
    for (const auto &[id, node] : graph.getNodes())
        if (node.kind == NodeKind::OnWall)
            onWalls.push_back(node);
    return onWalls;
}

inline std::set<std::pair<int, int>> rowsJoinedByClimbing(
    const NavigationGraph &graph,
    const TileMap &tileMap)
{
    std::set<std::pair<int, int>> joined;
    for (const auto &[id, node] : graph.getNodes())
        for (const NavigationEdge &edge : graph.getOutgoingEdges(id))
        {
            if (edge.type != EdgeType::Climb)
                continue;

            glm::vec2 underfoot(0.0f, 1.0f);
            int from = tileMap.tileContaining(graph.getNode(edge.fromId).feet + underfoot).y;
            int to = tileMap.tileContaining(graph.getNode(edge.toId).feet + underfoot).y;
            joined.insert({from, to});
        }
    return joined;
}
