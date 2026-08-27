#pragma once

#include <string>
#include "navigation/navigation_graph.hpp"
#include "navigation/navigation_profile.hpp"

struct NamedNavigationGraph
{
    std::string name;
    NavigationProfile profile;
    NavigationGraph graph;
};
