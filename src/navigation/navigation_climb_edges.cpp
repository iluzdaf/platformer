#include <algorithm>
#include <cstddef>
#include <optional>
#include <vector>
#include "navigation/navigation_graph_steps.hpp"
#include "navigation/jump_simulation.hpp"
#include "navigation/navigation_profile.hpp"
#include "tile_map/tile.hpp"
#include "navigation/navigation_graph.hpp"
#include "navigation/navigation_node.hpp"
#include "navigation/navigation_edge.hpp"
#include "tile_map/tile_map.hpp"
#include "physics/aabb.hpp"

namespace
{
    bool grippableBeside(const TileMap &tileMap, int wallX, int footRow, int headroom)
    {
        for (int offset = 1; offset <= headroom; ++offset)
        {
            glm::ivec2 beside(wallX, footRow - offset);
            if (!tileMap.validTilePosition(beside) ||
                !tileMap.getTileAtTilePosition(beside).isGrippable())
                return false;
        }

        return true;
    }

    bool canHangAt(const TileMap &tileMap, int climbX, int wallX, int footRow, int headroom)
    {
        return navigation::canStandOn(tileMap, glm::ivec2(climbX, footRow), headroom) &&
               grippableBeside(tileMap, wallX, footRow, headroom);
    }

    std::optional<int> ledgeAboveTheFace(
        const NavigationGraph &navigationGraph,
        const TileMap &tileMap,
        int climbX,
        int wallX,
        int runTop,
        int headroom,
        float stepHeight)
    {
        glm::ivec2 wallTop(wallX, runTop - headroom);
        glm::ivec2 above = wallTop + glm::ivec2(0, -1);
        if (tileMap.validTilePosition(above) && tileMap.getTileAtTilePosition(above).isSolid())
            return std::nullopt;

        return navigation::nodeGoverning(
            navigationGraph,
            tileMap,
            navigation::againstTheWall(tileMap, wallX, climbX, wallTop.y),
            headroom,
            stepHeight);
    }
}

namespace navigation
{
    glm::vec2 againstTheWall(const TileMap &tileMap, int climbX, int wallX, int footRow)
    {
        float tileSize = static_cast<float>(tileMap.getTileSize());
        glm::vec2 corner = tileMap.topLeftOfTile(glm::ivec2(climbX, footRow));
        return corner + glm::vec2(wallX > climbX ? tileSize : 0.0f, 0.0f);
    }

    std::vector<ClimbFace> addClimbing(
        NavigationGraph &navigationGraph,
        const TileMap &tileMap,
        const NavigationProfile &profile,
        int headroom)
    {
        std::vector<ClimbFace> faces;
        if (!profile.climbs())
            return faces;

        int nextNodeId = 0;
        for (const auto &[id, node] : navigationGraph.getNodes())
            nextNodeId = std::max(nextNodeId, id + 1);

        auto join = [&](int fromId, int toId, float wallDirection)
        {
            if (fromId != toId)
                navigationGraph.addEdge(timed(
                    {fromId, toId, EdgeType::Climb, {}, {}, wallDirection},
                    navigationGraph,
                    profile));
        };

        auto joinBothWays = [&](int fromId, int toId, float wallDirection)
        {
            join(fromId, toId, wallDirection);
            join(toId, fromId, wallDirection);
        };

        auto endOfTheFace = [&](int climbX, int wallX, int footRow)
        {
            glm::vec2 position = againstTheWall(tileMap, climbX, wallX, footRow);
            if (std::optional<AABB> ground = tileMap.groundAt(glm::ivec2(climbX, footRow)))
                position = glm::vec2(
                    std::clamp(position.x, ground->left(), ground->right()), ground->top());

            std::optional<int> existing = navigationGraph.nodeAtPosition(position);
            if (existing)
                return *existing;

            int id = nextNodeId++;
            navigationGraph.addNode(id, position, NodeKind::OnWall);
            return id;
        };

        for (int climbX = 0; climbX < tileMap.getWidth(); ++climbX)
            for (int side : {-1, 1})
            {
                int wallX = climbX + side;
                std::optional<int> runTop;

                for (int footRow = 0; footRow <= tileMap.getHeight(); ++footRow)
                {
                    if (footRow < tileMap.getHeight() &&
                        canHangAt(tileMap, climbX, wallX, footRow, headroom))
                    {
                        if (!runTop)
                            runTop = footRow;
                        continue;
                    }

                    if (!runTop)
                        continue;

                    int runBottom = footRow - 1;
                    std::optional<int> ledgeId = ledgeAboveTheFace(
                        navigationGraph,
                        tileMap,
                        climbX,
                        wallX,
                        *runTop,
                        headroom,
                        profile.physicsBodyData.stepHeight);

                    if (runBottom == *runTop && !ledgeId)
                    {
                        runTop.reset();
                        continue;
                    }

                    int topId = endOfTheFace(climbX, wallX, *runTop);
                    int bottomId = endOfTheFace(climbX, wallX, runBottom);
                    float wallDirection = static_cast<float>(side);
                    joinBothWays(topId, bottomId, wallDirection);
                    faces.push_back({climbX, wallX, *runTop, runBottom, topId, bottomId});
                    if (ledgeId)
                        join(topId, *ledgeId, wallDirection);

                    if (ledgeId && profile.abilities.lower)
                        join(*ledgeId, topId, wallDirection);

                    runTop.reset();
                }
            }

        return faces;
    }
}
