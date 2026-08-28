#pragma once

#include <vector>

class NavigationGraph;

// The cheapest route from one node to another, as the nodes to visit in order,
// starting at fromId and ending at toId. Empty when there is no route. Throws
// if either node is not in the graph.
std::vector<int> findPath(const NavigationGraph &navigationGraph, int fromId, int toId);

// The nodes that can be reached from a node and got back from again, in id
// order and including the node itself. Leaves out anything past a one way drop.
// A patrol is told where to walk rather than working this out for itself, so
// this is for asking whether a level, or a beat someone has authored, can be
// walked both ways.
std::vector<int> roundTripFrom(const NavigationGraph &navigationGraph, int fromId);

// The nodes reachable from a node on foot alone, in id order and including the
// node itself. These are the nodes sharing its platform.
std::vector<int> walkableFrom(const NavigationGraph &navigationGraph, int fromId);
