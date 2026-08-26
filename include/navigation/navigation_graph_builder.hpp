#pragma once

#include "navigation/navigation_graph.hpp"
#include "navigation/navigation_profile.hpp"

class TileMap;

NavigationGraph buildNavigationGraph(
    const TileMap &tileMap,
    const NavigationProfile &profile);
