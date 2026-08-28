#pragma once

#include <vector>

class NavigationGraph;

std::vector<int> findPath(const NavigationGraph &navigationGraph, int fromId, int toId);

std::vector<int> roundTripFrom(const NavigationGraph &navigationGraph, int fromId);

std::vector<int> walkableFrom(const NavigationGraph &navigationGraph, int fromId);
