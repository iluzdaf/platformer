#include <algorithm>
#include <cstddef>
#include <optional>
#include <utility>
#include <cmath>
#include <vector>
#include "navigation/navigation_graph_steps.hpp"
#include "navigation/jump_simulation.hpp"
#include "navigation/navigation_profile.hpp"
#include "tile_map/tile.hpp"
#include "navigation/navigation_graph.hpp"
#include "tile_map/tile_map.hpp"
#include "navigation/navigation_graph_builder.hpp"

namespace
{
    constexpr float NodeTileNudge = 0.5f;

    int tilesOfHeadroom(const TileMap &tileMap, const NavigationProfile &profile)
    {
        float tileSize = static_cast<float>(tileMap.getTileSize());
        return static_cast<int>(std::ceil(profile.colliderSize.y / tileSize));
    }

    void addNodes(NavigationGraph &navigationGraph, const TileMap &tileMap, int headroom)
    {
        int nextNodeId = 0;

        for (int y = 0; y < tileMap.getHeight(); ++y)
        {
            for (int x = 0; x < tileMap.getWidth(); ++x)
            {
                glm::ivec2 tilePosition(x, y);
                if (!tileMap.getTileAtTilePosition(tilePosition).isSolid())
                    continue;

                bool canWalkAbove = navigation::canStandOn(tileMap, tilePosition, headroom);
                if (!canWalkAbove)
                    continue;

                glm::ivec2 tilePositionLeft = tilePosition + glm::ivec2(-1, 0);
                bool isLeftCliff = tileMap.validTilePosition(tilePositionLeft) &&
                                   !tileMap.getTileAtTilePosition(tilePositionLeft).isSolid();
                glm::ivec2 tilePositionRight = tilePosition + glm::ivec2(1, 0);
                bool isRightCliff = tileMap.validTilePosition(tilePositionRight) &&
                                    !tileMap.getTileAtTilePosition(tilePositionRight).isSolid();

                bool canWalkAboveLeft =
                    !isLeftCliff && navigation::canStandOn(tileMap, tilePositionLeft, headroom);
                bool canWalkAboveRight =
                    !isRightCliff && navigation::canStandOn(tileMap, tilePositionRight, headroom);

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
}

namespace navigation
{
    bool canStandOn(const TileMap &tileMap, glm::ivec2 groundTilePosition, int headroom)
    {
        for (int offset = 1; offset <= headroom; ++offset)
        {
            glm::ivec2 above = groundTilePosition + glm::ivec2(0, -offset);
            if (!tileMap.validTilePosition(above))
                return false;

            const Tile &tile = tileMap.getTileAtTilePosition(above);
            if (tile.isSolid() || tile.isDeadly())
                return false;
        }

        return true;
    }

    bool isWalkableBetween(const TileMap &tileMap, glm::vec2 start, glm::vec2 end, int headroom)
    {
        if (start == end)
            return false;

        glm::vec2 underfoot(0.0f, 1.0f);
        glm::vec2 inwards = glm::normalize(end - start) * NodeTileNudge;
        glm::ivec2 startTilePosition = tileMap.worldToTilePosition(start + inwards + underfoot);
        glm::ivec2 endTilePosition = tileMap.worldToTilePosition(end - inwards + underfoot);

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

    bool clearAt(const TileMap &tileMap, glm::vec2 feetPosition, const NavigationProfile &profile)
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
                if (tile.isSolid() || tile.isDeadly())
                    return false;
            }

        return true;
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
}

NavigationGraph buildNavigationGraph(const TileMap &tileMap, const NavigationProfile &profile)
{
    NavigationGraph navigationGraph;
    int headroom = tilesOfHeadroom(tileMap, profile);

    addNodes(navigationGraph, tileMap, headroom);
    navigation::addFallLandingNodes(navigationGraph, tileMap, profile, headroom);
    navigation::addTakeOffNodes(navigationGraph, tileMap, profile, headroom);
    navigation::addWalkEdges(navigationGraph, tileMap, headroom);

    std::vector<navigation::ChosenJump> jumps =
        navigation::chooseJumps(navigationGraph, tileMap, profile, headroom);
    navigation::addJumpLandingNodes(navigationGraph, jumps);

    navigationGraph.clearEdges();
    navigation::addWalkEdges(navigationGraph, tileMap, headroom);

    navigation::addJumpEdges(navigationGraph, tileMap, headroom, jumps);
    navigation::addFallEdges(navigationGraph, tileMap, profile, headroom);

    navigation::addClimbing(navigationGraph, tileMap, profile, headroom);

    return navigationGraph;
}
