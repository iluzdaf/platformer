#include <algorithm>
#include <optional>
#include <set>
#include <utility>
#include <cmath>
#include <unordered_map>
#include <vector>
#include "navigation/navigation_graph_builder.hpp"
#include "game/tile_map/tile_map.hpp"

namespace
{
    constexpr float NodeTileNudge = 0.5f;

    int tilesOfHeadroom(const TileMap &tileMap, const NavigationProfile &profile)
    {
        float tileSize = static_cast<float>(tileMap.getTileSize());
        return static_cast<int>(std::ceil(profile.colliderSize.y / tileSize));
    }

    bool canStandOn(const TileMap &tileMap, glm::ivec2 groundTilePosition, int headroom)
    {
        for (int offset = 1; offset <= headroom; ++offset)
        {
            glm::ivec2 above = groundTilePosition + glm::ivec2(0, -offset);
            if (!tileMap.validTilePosition(above))
                return false;

            const Tile &tile = tileMap.getTileAtTilePosition(above);
            if (tile.isSolid() || tile.isSpikes())
                return false;
        }

        return true;
    }

    bool isWalkableBetween(
        const TileMap &tileMap,
        glm::vec2 start,
        glm::vec2 end,
        int headroom)
    {
        if (start == end)
            return false;

        glm::vec2 inwards = glm::normalize(end - start) * NodeTileNudge;
        glm::ivec2 startTilePosition = tileMap.worldToTilePosition(start + inwards);
        glm::ivec2 endTilePosition = tileMap.worldToTilePosition(end - inwards);

        if (startTilePosition.y != endTilePosition.y)
            return false;

        int fromX = std::min(startTilePosition.x, endTilePosition.x);
        int toX = std::max(startTilePosition.x, endTilePosition.x);

        for (int x = fromX; x <= toX; ++x)
        {
            glm::ivec2 groundTilePosition(x, startTilePosition.y);
            if (!tileMap.validTilePosition(groundTilePosition) ||
                !tileMap.getTileAtTilePosition(groundTilePosition).isSolid())
                return false;

            if (!canStandOn(tileMap, groundTilePosition, headroom))
                return false;
        }

        return true;
    }

    void addNodes(
        NavigationGraph &navigationGraph,
        const TileMap &tileMap,
        int headroom)
    {
        int nextNodeId = 0;

        for (int y = 0; y < tileMap.getHeight(); ++y)
        {
            for (int x = 0; x < tileMap.getWidth(); ++x)
            {
                glm::ivec2 tilePosition(x, y);
                if (!tileMap.getTileAtTilePosition(tilePosition).isSolid())
                    continue;

                bool canWalkAbove = canStandOn(tileMap, tilePosition, headroom);
                if (!canWalkAbove)
                    continue;

                glm::ivec2 tilePositionLeft = tilePosition + glm::ivec2(-1, 0);
                bool isLeftCliff = tileMap.validTilePosition(tilePositionLeft) &&
                                   !tileMap.getTileAtTilePosition(tilePositionLeft).isSolid();
                glm::ivec2 tilePositionRight = tilePosition + glm::ivec2(1, 0);
                bool isRightCliff = tileMap.validTilePosition(tilePositionRight) &&
                                    !tileMap.getTileAtTilePosition(tilePositionRight).isSolid();

                bool canWalkAboveLeft = !isLeftCliff &&
                                        canStandOn(tileMap, tilePositionLeft, headroom);
                bool canWalkAboveRight = !isRightCliff &&
                                         canStandOn(tileMap, tilePositionRight, headroom);

                if (!isLeftCliff && !isRightCliff && canWalkAboveLeft && canWalkAboveRight)
                    continue;

                glm::vec2 worldPosition = tileMap.tileToWorldPosition(tilePosition);
                float tileSize = static_cast<float>(tileMap.getTileSize());
                glm::vec2 nodeOffset(tileSize / 2.0f, 0.0f);

                if (canWalkAboveLeft && !canWalkAboveRight)
                    nodeOffset = glm::vec2(tileSize, 0.0f);
                else if (!canWalkAboveLeft && canWalkAboveRight)
                    nodeOffset = glm::vec2(0.0f, 0.0f);

                navigationGraph.addNode(nextNodeId++, worldPosition + nodeOffset);
            }
        }
    }

    void addWalkEdges(
        NavigationGraph &navigationGraph,
        const TileMap &tileMap,
        int headroom)
    {
        std::unordered_map<int, std::vector<NavigationNode>> nodesByRow;

        for (const auto &[id, node] : navigationGraph.getNodes())
            nodesByRow[static_cast<int>(std::round(node.position.y))].push_back(node);

        std::vector<int> rows;
        for (const auto &[y, nodesInRow] : nodesByRow)
            rows.push_back(y);
        std::sort(rows.begin(), rows.end());

        for (int y : rows)
        {
            std::vector<NavigationNode> &nodesInRow = nodesByRow[y];
            std::sort(
                nodesInRow.begin(),
                nodesInRow.end(),
                [](const NavigationNode &left, const NavigationNode &right)
                { return left.position.x < right.position.x; });

            for (size_t index = 1; index < nodesInRow.size(); ++index)
            {
                const NavigationNode &left = nodesInRow[index - 1];
                const NavigationNode &right = nodesInRow[index];

                if (!isWalkableBetween(tileMap, left.position, right.position, headroom))
                    continue;

                navigationGraph.addEdge(left.id, right.id, EdgeType::Walk);
                navigationGraph.addEdge(right.id, left.id, EdgeType::Walk);
            }
        }
    }

    bool clearAt(
        const TileMap &tileMap,
        glm::vec2 feetPosition,
        const NavigationProfile &profile)
    {
        constexpr float Inset = 0.5f;
        float halfWidth = profile.colliderSize.x * 0.5f - Inset;
        glm::vec2 low(feetPosition.x - halfWidth, feetPosition.y - profile.colliderSize.y + Inset);
        glm::vec2 high(feetPosition.x + halfWidth, feetPosition.y - Inset);

        glm::ivec2 lowTilePosition = tileMap.worldToTilePosition(low);
        glm::ivec2 highTilePosition = tileMap.worldToTilePosition(high);

        for (int y = lowTilePosition.y; y <= highTilePosition.y; ++y)
            for (int x = lowTilePosition.x; x <= highTilePosition.x; ++x)
            {
                glm::ivec2 tilePosition(x, y);
                if (!tileMap.validTilePosition(tilePosition))
                    return false;

                const Tile &tile = tileMap.getTileAtTilePosition(tilePosition);
                if (tile.isSolid() || tile.isSpikes())
                    return false;
            }

        return true;
    }

    std::optional<glm::vec2> landingOf(
        const TileMap &tileMap,
        glm::vec2 takeOff,
        const std::vector<glm::vec2> &arc,
        float direction,
        const NavigationProfile &profile)
    {
        float previousY = takeOff.y;

        for (size_t index = 1; index < arc.size(); ++index)
        {
            glm::vec2 position = takeOff + glm::vec2(direction * arc[index].x, arc[index].y);
            bool descending = position.y >= previousY;
            previousY = position.y;

            if (descending)
            {
                glm::ivec2 underfoot = tileMap.worldToTilePosition(position + glm::vec2(0.0f, 1.0f));
                if (tileMap.validTilePosition(underfoot) &&
                    tileMap.getTileAtTilePosition(underfoot).isSolid())
                    return glm::vec2(
                        position.x,
                        static_cast<float>(underfoot.y * tileMap.getTileSize()));
            }

            if (!clearAt(tileMap, position, profile))
                return std::nullopt;
        }

        return std::nullopt;
    }

    std::optional<int> nodeGoverning(
        const NavigationGraph &navigationGraph,
        const TileMap &tileMap,
        glm::vec2 landing,
        int headroom)
    {
        constexpr float SameSurface = 0.5f;
        std::optional<int> nearest;
        float nearestDistance = 0.0f;

        for (const auto &[id, node] : navigationGraph.getNodes())
        {
            if (std::abs(node.position.y - landing.y) > SameSurface)
                continue;

            float distance = std::abs(node.position.x - landing.x);
            if (distance > SameSurface &&
                !isWalkableBetween(tileMap, landing, node.position, headroom))
                continue;

            if (nearest && distance >= nearestDistance)
                continue;

            nearest = id;
            nearestDistance = distance;
        }

        return nearest;
    }

    bool alreadyWalkable(const NavigationGraph &navigationGraph, int fromId, int toId)
    {
        for (const auto &edge : navigationGraph.getOutgoingEdges(fromId))
            if (edge.type == EdgeType::Walk && edge.toId == toId)
                return true;

        return false;
    }

    void addJumpEdges(
        NavigationGraph &navigationGraph,
        const TileMap &tileMap,
        const NavigationProfile &profile,
        int headroom)
    {
        std::set<std::pair<int, int>> added;
        std::vector<std::pair<int, glm::vec2>> takeOffs;
        for (const auto &[id, node] : navigationGraph.getNodes())
            takeOffs.emplace_back(id, node.position);

        for (const auto &[fromId, takeOff] : takeOffs)
            for (const std::vector<glm::vec2> &arc : profile.jumpArcs)
                for (float direction : {1.0f, -1.0f})
                {
                    std::optional<glm::vec2> landing =
                        landingOf(tileMap, takeOff, arc, direction, profile);
                    if (!landing || landing->y > takeOff.y)
                        continue;

                    std::optional<int> toId =
                        nodeGoverning(navigationGraph, tileMap, *landing, headroom);
                    if (!toId || *toId == fromId)
                        continue;

                    if (alreadyWalkable(navigationGraph, fromId, *toId))
                        continue;

                    if (added.insert({fromId, *toId}).second)
                        navigationGraph.addEdge(fromId, *toId, EdgeType::Jump);
                }
    }
}

NavigationGraph buildNavigationGraph(
    const TileMap &tileMap,
    const NavigationProfile &profile)
{
    NavigationGraph navigationGraph;
    int headroom = tilesOfHeadroom(tileMap, profile);

    addNodes(navigationGraph, tileMap, headroom);
    addWalkEdges(navigationGraph, tileMap, headroom);
    addJumpEdges(navigationGraph, tileMap, profile, headroom);

    return navigationGraph;
}
