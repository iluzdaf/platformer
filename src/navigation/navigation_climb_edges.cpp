#include <algorithm>
#include <cstddef>
#include <optional>
#include "navigation/navigation_graph_steps.hpp"
#include "navigation/jump_simulation.hpp"
#include "navigation/navigation_profile.hpp"
#include "tile_map/tile.hpp"
#include "navigation/navigation_graph.hpp"
#include "navigation/navigation_node.hpp"
#include "navigation/navigation_edge.hpp"
#include "tile_map/tile_map.hpp"

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

    glm::vec2 againstTheWall(const TileMap &tileMap, int climbX, int wallX, int footRow)
    {
        float tileSize = static_cast<float>(tileMap.getTileSize());
        glm::vec2 corner = tileMap.topLeftOfTile(glm::ivec2(climbX, footRow));
        return corner + glm::vec2(wallX > climbX ? tileSize : 0.0f, 0.0f);
    }

    std::optional<int> ledgeAboveTheFace(
        const NavigationGraph &navigationGraph,
        const TileMap &tileMap,
        int climbX,
        int wallX,
        int runTop,
        int headroom)
    {
        glm::ivec2 wallTop(wallX, runTop - headroom);
        glm::ivec2 above = wallTop + glm::ivec2(0, -1);
        if (tileMap.validTilePosition(above) && tileMap.getTileAtTilePosition(above).isSolid())
            return std::nullopt;

        return navigation::nodeGoverning(
            navigationGraph, tileMap, againstTheWall(tileMap, wallX, climbX, wallTop.y), headroom);
    }
}

namespace navigation
{
    void addClimbing(
        NavigationGraph &navigationGraph,
        const TileMap &tileMap,
        const NavigationProfile &profile,
        int headroom)
    {
        if (!profile.climbs())
            return;

        int nextNodeId = 0;
        for (const auto &[id, node] : navigationGraph.getNodes())
            nextNodeId = std::max(nextNodeId, id + 1);

        auto joinBothWays = [&](int fromId, int toId, float wallDirection)
        {
            if (fromId == toId)
                return;

            navigationGraph.addEdge(
                NavigationEdge{fromId, toId, EdgeType::Climb, {}, 0.0f, wallDirection});
            navigationGraph.addEdge(
                NavigationEdge{toId, fromId, EdgeType::Climb, {}, 0.0f, wallDirection});
        };

        auto endOfTheFace = [&](int climbX, int wallX, int footRow)
        {
            glm::vec2 position = againstTheWall(tileMap, climbX, wallX, footRow);
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
                        navigationGraph, tileMap, climbX, wallX, *runTop, headroom);

                    if (runBottom == *runTop && !ledgeId)
                    {
                        runTop.reset();
                        continue;
                    }

                    int topId = endOfTheFace(climbX, wallX, *runTop);
                    float wallDirection = static_cast<float>(side);
                    joinBothWays(topId, endOfTheFace(climbX, wallX, runBottom), wallDirection);
                    if (ledgeId)
                        joinBothWays(topId, *ledgeId, wallDirection);

                    runTop.reset();
                }
            }
    }
}
