#include <algorithm>
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
}

NavigationGraph buildNavigationGraph(
    const TileMap &tileMap,
    const NavigationProfile &profile)
{
    NavigationGraph navigationGraph;
    int headroom = tilesOfHeadroom(tileMap, profile);

    addNodes(navigationGraph, tileMap, headroom);
    addWalkEdges(navigationGraph, tileMap, headroom);

    return navigationGraph;
}
