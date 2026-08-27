#pragma once

#include <vector>

class NavigationGraph;

// The cheapest route from one node to another, as the nodes to visit in order,
// starting at fromId and ending at toId. Empty when there is no route. Throws
// if either node is not in the graph.
std::vector<int> findPath(const NavigationGraph &navigationGraph, int fromId, int toId);
