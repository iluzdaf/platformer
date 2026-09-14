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
#include "physics/aabb.hpp"
#include "navigation/navigation_graph_builder.hpp"

namespace
{
    constexpr float NodeTileNudge = 0.5f;

    int tilesOfHeadroom(const TileMap &tileMap, const NavigationProfile &profile)
    {
        float tileSize = static_cast<float>(tileMap.getTileSize());
        return static_cast<int>(std::ceil(profile.physicsBodyData.colliderSize.y / tileSize));
    }

    void addRunEnds(
        NavigationGraph &navigationGraph,
        const TileMap &tileMap,
        int headroom,
        float stepHeight)
    {
        int nextNodeId = 0;

        for (int y = 0; y < tileMap.getHeight(); ++y)
        {
            for (int x = 0; x < tileMap.getWidth(); ++x)
            {
                glm::ivec2 tilePosition(x, y);
                std::optional<AABB> ground = tileMap.groundAt(tilePosition);
                if (!ground || !navigation::canStandOn(tileMap, tilePosition, headroom))
                    continue;

                auto walksOnto = [&](glm::ivec2 neighbour)
                {
                    return navigation::canStandOn(tileMap, neighbour, headroom) &&
                           navigation::stepsBetween(tileMap, tilePosition, neighbour, stepHeight);
                };
                bool walksLeft = walksOnto(tilePosition + glm::ivec2(-1, 0));
                bool walksRight = walksOnto(tilePosition + glm::ivec2(1, 0));
                if (walksLeft && walksRight)
                    continue;

                float feetX = walksLeft    ? ground->right()
                              : walksRight ? ground->left()
                                           : ground->center().x;
                navigationGraph.addNode(nextNodeId++, glm::vec2(feetX, ground->top()));
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

    int groundRowOf(const TileMap &tileMap, glm::vec2 feet)
    {
        return tileMap.tileContaining(feet + glm::vec2(0.0f, 1.0f)).y;
    }

    glm::ivec2 groundUnder(const TileMap &tileMap, glm::vec2 feet, float towards)
    {
        constexpr float Settle = 0.5f;
        int row = groundRowOf(tileMap, feet);
        glm::vec2 nudge(towards < 0.0f ? -NodeTileNudge : NodeTileNudge, 0.0f);
        glm::ivec2 inwards(tileMap.tileContaining(feet + nudge).x, row);
        glm::ivec2 outwards(tileMap.tileContaining(feet - nudge).x, row);

        for (glm::ivec2 tilePosition : {inwards, outwards})
        {
            std::optional<AABB> ground = tileMap.groundAt(tilePosition);
            if (ground && std::abs(ground->top() - feet.y) <= Settle)
                return tilePosition;
        }

        return inwards;
    }

    bool feetOverGround(const TileMap &tileMap, glm::vec2 feet)
    {
        constexpr float Settle = 0.5f;
        int row = groundRowOf(tileMap, feet);
        for (float across : {-Settle, Settle})
        {
            glm::ivec2 under(tileMap.tileContaining(feet + glm::vec2(across, 0.0f)).x, row);
            std::optional<AABB> ground = tileMap.groundAt(under);
            if (ground && feet.x >= ground->left() - Settle && feet.x <= ground->right() + Settle)
                return true;
        }

        return false;
    }

    bool stepsBetween(const TileMap &tileMap, glm::ivec2 from, glm::ivec2 to, float stepHeight)
    {
        std::optional<AABB> here = tileMap.groundAt(from);
        std::optional<AABB> there = tileMap.groundAt(to);

        return here && there && std::abs(here->top() - there->top()) <= stepHeight;
    }

    bool isWalkableBetween(
        const TileMap &tileMap,
        glm::vec2 start,
        glm::vec2 end,
        int headroom,
        float stepHeight)
    {
        if (start == end)
            return false;

        glm::ivec2 startTilePosition = groundUnder(tileMap, start, end.x - start.x);
        glm::ivec2 endTilePosition = groundUnder(tileMap, end, start.x - end.x);

        if (startTilePosition.y != endTilePosition.y)
            return false;

        int fromX = std::min(startTilePosition.x, endTilePosition.x);
        int toX = std::max(startTilePosition.x, endTilePosition.x);

        for (int x = fromX; x <= toX; ++x)
        {
            glm::ivec2 groundTilePosition(x, startTilePosition.y);
            if (!tileMap.groundAt(groundTilePosition))
                return false;

            if (!canStandOn(tileMap, groundTilePosition, headroom))
                return false;

            glm::ivec2 before = groundTilePosition - glm::ivec2(1, 0);
            if (x > fromX && !stepsBetween(tileMap, before, groundTilePosition, stepHeight))
                return false;
        }

        return true;
    }

    bool clearAt(const TileMap &tileMap, glm::vec2 feetPosition, const NavigationProfile &profile)
    {
        constexpr float Inset = 0.5f;
        glm::vec2 size = profile.physicsBodyData.colliderSize;
        float stepHeight = profile.physicsBodyData.stepHeight;
        AABB body(
            glm::vec2(feetPosition.x - size.x * 0.5f + Inset, feetPosition.y - size.y + Inset),
            glm::vec2(size.x - Inset * 2.0f, size.y - Inset * 2.0f - stepHeight));

        glm::ivec2 lowTilePosition = tileMap.tileContaining(body.position);
        glm::ivec2 highTilePosition = tileMap.tileContaining(body.position + body.size);

        for (int y = lowTilePosition.y; y <= highTilePosition.y; ++y)
            for (int x = lowTilePosition.x; x <= highTilePosition.x; ++x)
            {
                glm::ivec2 tilePosition(x, y);
                if (!tileMap.validTilePosition(tilePosition))
                    return false;

                if (tileMap.getTileAtTilePosition(tilePosition).isDeadly())
                    return false;

                std::optional<AABB> ground = tileMap.groundAt(tilePosition);
                if (ground && ground->intersects(body))
                    return false;
            }

        return true;
    }

    std::optional<int> nodeGoverning(
        const NavigationGraph &navigationGraph,
        const TileMap &tileMap,
        glm::vec2 landing,
        int headroom,
        float stepHeight)
    {
        constexpr float SameSurface = 0.5f;
        std::optional<int> nearest;
        float nearestDistance = 0.0f;

        for (const auto &[id, node] : navigationGraph.getNodes())
        {
            if (groundRowOf(tileMap, node.feet) != groundRowOf(tileMap, landing) ||
                std::abs(node.feet.y - landing.y) > stepHeight)
                continue;

            float distance = std::abs(node.feet.x - landing.x);
            if (distance > SameSurface &&
                !isWalkableBetween(tileMap, landing, node.feet, headroom, stepHeight))
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
        glm::ivec2 column = tileMap.tileContaining(glm::vec2(x, below));

        for (int y = column.y; y < tileMap.getHeight(); ++y)
        {
            glm::ivec2 ground(column.x, y);
            if (!tileMap.validTilePosition(ground))
                return std::nullopt;

            std::optional<AABB> collider = tileMap.groundAt(ground);
            if (!collider || collider->top() <= below)
                continue;

            glm::vec2 standing(x, collider->top());
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

    float stepHeight = profile.physicsBodyData.stepHeight;

    addRunEnds(navigationGraph, tileMap, headroom, stepHeight);
    std::vector<navigation::ChosenFall> falls =
        navigation::addFallLandingNodes(navigationGraph, tileMap, profile, headroom);
    navigation::addJumpTakeOffNodes(navigationGraph, tileMap, profile, headroom);

    std::vector<navigation::ChosenJump> jumps =
        navigation::chooseJumps(navigationGraph, tileMap, profile, headroom);
    navigation::addJumpLandingNodes(navigationGraph, jumps);

    navigation::addWalkEdges(navigationGraph, tileMap, profile, headroom);
    navigation::addJumpEdges(navigationGraph, tileMap, profile, headroom, jumps);
    navigation::addFallEdges(navigationGraph, tileMap, profile, headroom, falls);

    navigation::addClimbing(navigationGraph, tileMap, profile, headroom);
    navigation::addWallJumps(navigationGraph, tileMap, profile, headroom);

    return navigationGraph;
}
