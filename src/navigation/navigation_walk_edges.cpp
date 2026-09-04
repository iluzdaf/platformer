#include <algorithm>
#include <cstddef>
#include <utility>
#include <cmath>
#include <unordered_map>
#include <vector>
#include "navigation/navigation_graph_steps.hpp"
#include "navigation/jump_simulation.hpp"
#include "navigation/navigation_graph.hpp"
#include "navigation/navigation_node.hpp"
#include "navigation/navigation_edge.hpp"
#include "tile_map/tile_map.hpp"

namespace navigation
{
    std::vector<std::vector<int>> walkRuns(
        const NavigationGraph &navigationGraph,
        const TileMap &tileMap,
        int headroom)
    {
        std::unordered_map<int, std::vector<NavigationNode>> nodesByRow;

        for (const auto &[id, node] : navigationGraph.getNodes())
            nodesByRow[static_cast<int>(std::round(node.position.y))].push_back(node);

        std::vector<int> rows;
        rows.reserve(nodesByRow.size());
        for (const auto &[y, nodesInRow] : nodesByRow)
            rows.push_back(y);
        std::sort(rows.begin(), rows.end());

        std::vector<std::vector<int>> runs;
        for (int y : rows)
        {
            std::vector<NavigationNode> &nodesInRow = nodesByRow[y];
            std::sort(
                nodesInRow.begin(),
                nodesInRow.end(),
                [](const NavigationNode &left, const NavigationNode &right)
                { return left.position.x < right.position.x; });

            runs.push_back({nodesInRow[0].id});
            for (size_t index = 1; index < nodesInRow.size(); ++index)
            {
                const NavigationNode &left = nodesInRow[index - 1];
                const NavigationNode &right = nodesInRow[index];

                if (!isWalkableBetween(tileMap, left.position, right.position, headroom))
                    runs.push_back({});

                runs.back().push_back(right.id);
            }
        }

        return runs;
    }

    void addWalkEdges(NavigationGraph &navigationGraph, const TileMap &tileMap, int headroom)
    {
        for (const std::vector<int> &run : walkRuns(navigationGraph, tileMap, headroom))
            for (size_t index = 1; index < run.size(); ++index)
            {
                navigationGraph.addEdge(run[index - 1], run[index], EdgeType::Walk);
                navigationGraph.addEdge(run[index], run[index - 1], EdgeType::Walk);
            }
    }
}
