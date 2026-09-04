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

    void addRunEnds(NavigationGraph &navigationGraph, const TileMap &tileMap, int headroom)
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

                glm::vec2 worldPosition = tileMap.topLeftOfTile(tilePosition);
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
        glm::ivec2 startTilePosition = tileMap.tileContaining(start + inwards + underfoot);
        glm::ivec2 endTilePosition = tileMap.tileContaining(end - inwards + underfoot);

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

        glm::ivec2 lowTilePosition = tileMap.tileContaining(low);
        glm::ivec2 highTilePosition = tileMap.tileContaining(high);

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

    std::optional<glm::vec2> standingBelow(
        const TileMap &tileMap,
        float x,
        float below,
        const NavigationProfile &profile,
        int headroom)
    {
        float tileSize = static_cast<float>(tileMap.getTileSize());
        glm::ivec2 column = tileMap.tileContaining(glm::vec2(x, below));

        for (int y = column.y + 1; y < tileMap.getHeight(); ++y)
        {
            glm::ivec2 ground(column.x, y);
            if (!tileMap.validTilePosition(ground))
                return std::nullopt;
            if (!tileMap.getTileAtTilePosition(ground).isSolid())
                continue;

            glm::vec2 standing(x, static_cast<float>(y) * tileSize);
            if (canStandOn(tileMap, ground, headroom) && clearAt(tileMap, standing, profile))
                return standing;

            return std::nullopt;
        }

        return std::nullopt;
    }
}

NavigationGraph buildNavigationGraph(const TileMap &tileMap, const NavigationProfile &profile)
{
    NavigationGraph navigationGraph;
    int headroom = tilesOfHeadroom(tileMap, profile);

    addRunEnds(navigationGraph, tileMap, headroom);
    navigation::addFallLandingNodes(navigationGraph, tileMap, profile, headroom);
    navigation::addJumpTakeOffNodes(navigationGraph, tileMap, profile, headroom);

    std::vector<navigation::ChosenJump> jumps =
        navigation::chooseJumps(navigationGraph, tileMap, profile, headroom);
    navigation::addJumpLandingNodes(navigationGraph, jumps);

    navigation::addWalkEdges(navigationGraph, tileMap, headroom);
    navigation::addJumpEdges(navigationGraph, tileMap, headroom, jumps);
    navigation::addFallEdges(navigationGraph, tileMap, profile, headroom);

    navigation::addClimbing(navigationGraph, tileMap, profile, headroom);

    return navigationGraph;
}
