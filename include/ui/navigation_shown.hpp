#pragma once

#include <cstddef>
#include <optional>
#include <utility>

struct NavigationShown
{
    std::optional<std::size_t> graphIndex;
    std::optional<int> nodeId;
    std::optional<std::pair<int, int>> edge;

    bool operator==(const NavigationShown &) const = default;
};

inline NavigationShown showingGraph(std::optional<std::size_t> graphIndex)
{
    return NavigationShown{graphIndex, std::nullopt, std::nullopt};
}

inline NavigationShown showingNode(NavigationShown shown, std::optional<int> nodeId)
{
    return NavigationShown{shown.graphIndex, nodeId, std::nullopt};
}

inline NavigationShown showingEdge(NavigationShown shown, std::optional<std::pair<int, int>> edge)
{
    return NavigationShown{shown.graphIndex, shown.nodeId, edge};
}

inline NavigationShown stillAmong(NavigationShown shown, std::size_t graphCount)
{
    if (shown.graphIndex && *shown.graphIndex >= graphCount)
        return NavigationShown{};

    return shown;
}
