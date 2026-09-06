#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <set>
#include <utility>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "helpers/graph_fixtures.hpp"
#include "navigation/navigation_edge.hpp"
#include "navigation/navigation_graph.hpp"
#include "navigation/navigation_node.hpp"
#include "tile_map/tile_map.hpp"

inline std::vector<float> nodeXsOnRow(const NavigationGraph &graph, float y)
{
    std::vector<float> xs;
    for (const auto &[id, node] : graph.getNodes())
        if (node.position.y == y)
            xs.push_back(node.position.x);
    std::sort(xs.begin(), xs.end());
    return xs;
}

inline int nodeIdAt(const NavigationGraph &graph, float x, float y)
{
    for (const auto &[id, node] : graph.getNodes())
        if (node.position == glm::vec2(x, y))
            return id;
    return -1;
}

inline int nodeAt(const NavigationGraph &graph, glm::vec2 position)
{
    for (const auto &[id, node] : graph.getNodes())
        if (glm::distance(node.position, position) < 0.5f)
            return id;

    return -1;
}

inline bool hasEdgeBetween(const NavigationGraph &graph, float fromX, float toX, float y)
{
    for (const auto &edge : graph.getEdges())
    {
        NavigationNode from = graph.getNode(edge.fromId);
        NavigationNode to = graph.getNode(edge.toId);
        if (from.position == glm::vec2(fromX, y) && to.position == glm::vec2(toX, y))
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

        if (glm::distance(graph.getNode(edge.fromId).position, from) > Tolerance)
            continue;

        if (glm::distance(graph.getNode(edge.toId).position, to) <= Tolerance)
            return true;
    }

    return false;
}

inline bool isReachable(const NavigationGraph &graph, glm::vec2 from, glm::vec2 to)
{
    int fromId = nodeIdAt(graph, from.x, from.y);
    int toId = nodeIdAt(graph, to.x, to.y);
    if (fromId < 0 || toId < 0)
        return false;

    std::set<int> seen{fromId};
    std::vector<int> pending{fromId};
    while (!pending.empty())
    {
        int nodeId = pending.back();
        pending.pop_back();
        if (nodeId == toId)
            return true;

        for (const auto &edge : graph.getOutgoingEdges(nodeId))
            if (seen.insert(edge.toId).second)
                pending.push_back(edge.toId);
    }
    return false;
}

inline size_t nodesOnTheFloor(const NavigationGraph &graph, const TileMap &tileMap)
{
    float floorY = static_cast<float>(FloorRow * tileMap.getTileSize());
    size_t count = 0;
    for (const auto &[id, node] : graph.getNodes())
        if (node.position.y == floorY)
            ++count;
    return count;
}

inline bool walksTheFloorEndToEnd(const NavigationGraph &graph, const TileMap &tileMap)
{
    float floorY = static_cast<float>(FloorRow * tileMap.getTileSize());
    for (const auto &edge : graph.getEdges())
    {
        if (edge.type != EdgeType::Walk)
            continue;

        NavigationNode from = graph.getNode(edge.fromId);
        NavigationNode to = graph.getNode(edge.toId);
        if (from.position.y == floorY && to.position.y == floorY)
            return true;
    }
    return false;
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

        if (glm::distance(graph.getNode(edge.fromId).position, from) > Tolerance)
            continue;

        NavigationNode to = graph.getNode(edge.toId);
        if (std::abs(to.position.y - farSide.y) > Tolerance)
            continue;

        bool overThere = farSide.x > from.x ? to.position.x >= farSide.x - Tolerance
                                            : to.position.x <= farSide.x + Tolerance;
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
            int from = tileMap.tileContaining(graph.getNode(edge.fromId).position + underfoot).y;
            int to = tileMap.tileContaining(graph.getNode(edge.toId).position + underfoot).y;
            joined.insert({from, to});
        }
    return joined;
}

inline int nodeJustPastTheLedge(const NavigationGraph &graph, float floorY)
{
    float ledgeEdgeX = static_cast<float>(LeftPlatformEnd + 1) * 16.0f;
    for (const auto &[id, node] : graph.getNodes())
        if (std::abs(node.position.y - floorY) < 0.5f && node.position.x > ledgeEdgeX &&
            node.position.x < ledgeEdgeX + 8.0f)
            return id;

    return -1;
}

inline bool anEdgeSpansThePinch(const NavigationGraph &graph, const TileMap &tileMap)
{
    float tileSize = static_cast<float>(tileMap.getTileSize());
    float pinchX = (static_cast<float>(PinchColumn) + 0.5f) * tileSize;
    float floorY = static_cast<float>(FloorRow) * tileSize;
    for (const auto &edge : graph.getEdges())
    {
        NavigationNode from = graph.getNode(edge.fromId);
        NavigationNode to = graph.getNode(edge.toId);
        if (from.position.y != floorY || to.position.y != floorY)
            continue;

        float low = std::min(from.position.x, to.position.x);
        float high = std::max(from.position.x, to.position.x);
        if (low < pinchX && high > pinchX)
            return true;
    }
    return false;
}
