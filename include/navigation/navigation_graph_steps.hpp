#pragma once

#include <optional>
#include <vector>
#include <glm/glm.hpp>

class NavigationGraph;
class TileMap;
struct NavigationProfile;

namespace navigation
{
    struct ChosenJump
    {
        int fromId = 0;
        std::vector<glm::vec2> path;
        float holdDuration = 0.0f;
    };

    bool canStandOn(const TileMap &tileMap, glm::ivec2 groundTilePosition, int headroom);

    bool isWalkableBetween(const TileMap &tileMap, glm::vec2 start, glm::vec2 end, int headroom);

    bool clearAt(const TileMap &tileMap, glm::vec2 feetPosition, const NavigationProfile &profile);

    std::optional<int> nodeGoverning(
        const NavigationGraph &navigationGraph,
        const TileMap &tileMap,
        glm::vec2 landing,
        int headroom);

    void addWalkEdges(NavigationGraph &navigationGraph, const TileMap &tileMap, int headroom);

    std::vector<ChosenJump> chooseJumps(
        NavigationGraph &navigationGraph,
        const TileMap &tileMap,
        const NavigationProfile &profile,
        int headroom);

    void addJumpLandingNodes(
        NavigationGraph &navigationGraph,
        const std::vector<ChosenJump> &jumps);

    void addJumpEdges(
        NavigationGraph &navigationGraph,
        const TileMap &tileMap,
        int headroom,
        const std::vector<ChosenJump> &jumps);

    void addFallLandingNodes(
        NavigationGraph &navigationGraph,
        const TileMap &tileMap,
        const NavigationProfile &profile,
        int headroom);

    void addTakeOffNodes(
        NavigationGraph &navigationGraph,
        const TileMap &tileMap,
        const NavigationProfile &profile,
        int headroom);

    void addFallEdges(
        NavigationGraph &navigationGraph,
        const TileMap &tileMap,
        const NavigationProfile &profile,
        int headroom);

    void addClimbing(
        NavigationGraph &navigationGraph,
        const TileMap &tileMap,
        const NavigationProfile &profile,
        int headroom);
}
