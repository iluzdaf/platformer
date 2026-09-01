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
    void addWalkEdges(NavigationGraph &navigationGraph, const TileMap &tileMap, int headroom)
    {
        std::unordered_map<int, std::vector<NavigationNode>> nodesByRow;

        for (const auto &[id, node] : navigationGraph.getNodes())
            nodesByRow[static_cast<int>(std::round(node.position.y))].push_back(node);

        std::vector<int> rows;
        rows.reserve(nodesByRow.size());
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
