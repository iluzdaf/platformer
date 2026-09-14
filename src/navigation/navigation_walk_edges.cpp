#include <algorithm>
#include <cstddef>
#include <utility>
#include <unordered_map>
#include <vector>
#include "navigation/navigation_graph_steps.hpp"
#include "navigation/jump_simulation.hpp"
#include "navigation/navigation_graph.hpp"
#include "navigation/navigation_node.hpp"
#include "navigation/navigation_edge.hpp"
#include "navigation/navigation_profile.hpp"
#include "tile_map/tile_map.hpp"

namespace navigation
{
    std::vector<std::vector<int>> walkRuns(
        const NavigationGraph &navigationGraph,
        const TileMap &tileMap,
        int headroom,
        float stepHeight)
    {
        std::unordered_map<int, std::vector<NavigationNode>> nodesByRow;

        for (const auto &[id, node] : navigationGraph.getNodes())
            nodesByRow[groundRowOf(tileMap, node.feet)].push_back(node);

        std::vector<int> rows;
        rows.reserve(nodesByRow.size());
        for (const auto &[y, nodesInRow] : nodesByRow)
            rows.push_back(y);
        std::sort(rows.begin(), rows.end());

        std::vector<std::vector<int>> runs;
        for (int y : rows)
        {
            std::vector<NavigationNode> &nodesInRow = nodesByRow[y];
            auto along = [&](const NavigationNode &node)
            { return std::pair(groundUnder(tileMap, node.feet, 1.0f).x, node.feet.x); };
            std::sort(
                nodesInRow.begin(),
                nodesInRow.end(),
                [&](const NavigationNode &left, const NavigationNode &right)
                { return along(left) < along(right); });

            runs.push_back({nodesInRow[0].id});
            for (size_t index = 1; index < nodesInRow.size(); ++index)
            {
                const NavigationNode &left = nodesInRow[index - 1];
                const NavigationNode &right = nodesInRow[index];

                if (!isWalkableBetween(tileMap, left.feet, right.feet, headroom, stepHeight))
                    runs.push_back({});

                runs.back().push_back(right.id);
            }
        }

        return runs;
    }

    void addWalkEdges(
        NavigationGraph &navigationGraph,
        const TileMap &tileMap,
        const NavigationProfile &profile,
        int headroom)
    {
        float stepHeight = profile.physicsBodyData.stepHeight;
        for (const std::vector<int> &run : walkRuns(navigationGraph, tileMap, headroom, stepHeight))
            for (size_t index = 1; index < run.size(); ++index)
            {
                auto walk = [&](int fromId, int toId)
                {
                    navigationGraph.addEdge(
                        timed({fromId, toId, EdgeType::Walk, {}, {}}, navigationGraph, profile));
                };
                walk(run[index - 1], run[index]);
                walk(run[index], run[index - 1]);
            }
    }
}
