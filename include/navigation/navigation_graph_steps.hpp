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

    int groundRowOf(const TileMap &tileMap, glm::vec2 feet);

    glm::ivec2 groundUnder(const TileMap &tileMap, glm::vec2 feet, float towards);

    bool feetOverGround(const TileMap &tileMap, glm::vec2 feet);

    bool stepsBetween(const TileMap &tileMap, glm::ivec2 from, glm::ivec2 to, float stepHeight);

    bool isWalkableBetween(
        const TileMap &tileMap,
        glm::vec2 start,
        glm::vec2 end,
        int headroom,
        float stepHeight);

    bool clearAt(const TileMap &tileMap, glm::vec2 feetPosition, const NavigationProfile &profile);

    std::optional<int> nodeGoverning(
        const NavigationGraph &navigationGraph,
        const TileMap &tileMap,
        glm::vec2 landing,
        int headroom,
        float stepHeight);

    std::optional<glm::vec2> standingBelow(
        const TileMap &tileMap,
        float x,
        float below,
        const NavigationProfile &profile,
        int headroom);

    std::vector<std::vector<int>> walkRuns(
        const NavigationGraph &navigationGraph,
        const TileMap &tileMap,
        int headroom,
        float stepHeight);

    void addWalkEdges(
        NavigationGraph &navigationGraph,
        const TileMap &tileMap,
        int headroom,
        float stepHeight);

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
        float stepHeight,
        const std::vector<ChosenJump> &jumps);

    void addFallLandingNodes(
        NavigationGraph &navigationGraph,
        const TileMap &tileMap,
        const NavigationProfile &profile,
        int headroom);

    void addJumpTakeOffNodes(
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
