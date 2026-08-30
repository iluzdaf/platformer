#include <algorithm>
#include <cstddef>
#include <optional>
#include <utility>
#include <vector>
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
    bool alreadyConnected(const NavigationGraph &navigationGraph, int fromId, int toId)
    {
        for (const auto &edge : navigationGraph.getOutgoingEdges(fromId))
            if (edge.toId == toId)
                return true;

        return false;
    }

    std::optional<glm::vec2> surfaceBelow(
        const TileMap &tileMap,
        glm::vec2 from,
        const NavigationProfile &profile,
        int headroom)
    {
        float tileSize = static_cast<float>(tileMap.getTileSize());
        glm::ivec2 startTilePosition = tileMap.worldToTilePosition(from);

        for (int y = startTilePosition.y; y < tileMap.getHeight(); ++y)
        {
            glm::ivec2 groundTilePosition(startTilePosition.x, y);
            if (!tileMap.validTilePosition(groundTilePosition))
                return std::nullopt;

            glm::vec2 standing(from.x, static_cast<float>(y) * tileSize);

            if (tileMap.getTileAtTilePosition(groundTilePosition).isSolid())
            {
                if (y == startTilePosition.y)
                    return std::nullopt;

                return navigation::canStandOn(tileMap, groundTilePosition, headroom) &&
                               navigation::clearAt(tileMap, standing, profile)
                           ? std::optional(standing)
                           : std::nullopt;
            }

            if (!navigation::clearAt(tileMap, standing, profile))
                return std::nullopt;
        }

        return std::nullopt;
    }

    std::vector<glm::vec2> fallLandings(
        const TileMap &tileMap,
        glm::vec2 takeOff,
        const NavigationProfile &profile,
        int headroom)
    {
        float stride = profile.colliderSize.x * 0.5f + 1.0f;

        std::vector<glm::vec2> landings;
        for (float direction : {-1.0f, 1.0f})
        {
            glm::vec2 steppedOff(takeOff.x + direction * stride, takeOff.y);
            if (std::optional<glm::vec2> landing =
                    surfaceBelow(tileMap, steppedOff, profile, headroom))
                landings.push_back(*landing);
        }

        return landings;
    }

}

namespace navigation
{
    void addFallLandingNodes(
        NavigationGraph &navigationGraph,
        const TileMap &tileMap,
        const NavigationProfile &profile,
        int headroom)
    {
        if (!profile.falls())
            return;

        int nextNodeId = 0;
        for (const auto &[id, node] : navigationGraph.getNodes())
            nextNodeId = std::max(nextNodeId, id + 1);

        for (bool added = true; added;)
        {
            added = false;
            std::vector<glm::vec2> takeOffs;
            for (const auto &[id, node] : navigationGraph.getNodes())
                takeOffs.push_back(node.position);

            for (glm::vec2 takeOff : takeOffs)
                for (glm::vec2 landing : fallLandings(tileMap, takeOff, profile, headroom))
                {
                    if (navigationGraph.hasNodeAtPosition(landing))
                        continue;

                    navigationGraph.addNode(nextNodeId++, landing, NodeKind::Landing);
                    added = true;
                }
        }
    }

    void addFallEdges(
        NavigationGraph &navigationGraph,
        const TileMap &tileMap,
        const NavigationProfile &profile,
        int headroom)
    {
        if (!profile.falls())
            return;

        std::vector<std::pair<int, glm::vec2>> takeOffs;
        for (const auto &[id, node] : navigationGraph.getNodes())
            takeOffs.emplace_back(id, node.position);

        for (const auto &[fromId, takeOff] : takeOffs)
            for (glm::vec2 landing : fallLandings(tileMap, takeOff, profile, headroom))
            {
                std::optional<int> toId =
                    nodeGoverning(navigationGraph, tileMap, landing, headroom);
                if (!toId || *toId == fromId)
                    continue;

                if (alreadyConnected(navigationGraph, fromId, *toId))
                    continue;

                navigationGraph.addEdge({fromId, *toId, EdgeType::Fall, {}, 0.0f});
            }
    }
}
