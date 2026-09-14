#include <algorithm>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "navigation/navigation_graph_steps.hpp"
#include "navigation/jump_simulation.hpp"
#include "navigation/navigation_profile.hpp"
#include "navigation/navigation_graph.hpp"
#include "navigation/navigation_build_report.hpp"
#include "navigation/navigation_node.hpp"
#include "navigation/navigation_edge.hpp"
#include "tile_map/tile_map.hpp"
#include "physics/physics_body.hpp"
#include "tile_map/tile.hpp"

namespace
{
    bool outermostOfItsRun(
        const NavigationGraph &navigationGraph,
        const std::unordered_map<int, int> &runs,
        int takeOffId,
        float direction)
    {
        float x = navigationGraph.getNode(takeOffId).feet.x;
        int run = runs.at(takeOffId);
        for (const auto &[id, other] : runs)
            if (other == run && (navigationGraph.getNode(id).feet.x - x) * direction > 0.0f)
                return false;

        return true;
    }

    std::vector<JumpAttempt> fallsFrom(
        NavigationGraph &navigationGraph,
        const TileMap &tileMap,
        const std::unordered_map<int, int> &runs,
        int takeOffId,
        const NavigationProfile &profile,
        int headroom)
    {
        std::vector<JumpAttempt> falls;
        if (!runs.contains(takeOffId))
            return falls;

        glm::vec2 takeOff = navigationGraph.getNode(takeOffId).feet;
        for (float direction : {-1.0f, 1.0f})
        {
            if (!outermostOfItsRun(navigationGraph, runs, takeOffId, direction))
                continue;

            JumpAttempt fall = simulateFallAgainst(
                tileMap, profile.abilities, profile.physicsBodyData, takeOff, direction);
            navigationGraph.building().noting(fall);
            if (!fall.landed)
                continue;

            glm::vec2 landing = fall.path.back();
            glm::ivec2 ground(
                tileMap.tileContaining(landing).x, navigation::groundRowOf(tileMap, landing));
            if (!navigation::feetOverGround(tileMap, landing) ||
                !navigation::canStandOn(tileMap, ground, headroom) ||
                navigation::touchesDeadly(tileMap, fall.path, profile))
                continue;

            falls.push_back(std::move(fall));
        }

        return falls;
    }
}

namespace navigation
{
    bool touchesDeadly(
        const TileMap &tileMap,
        const std::vector<glm::vec2> &path,
        const NavigationProfile &profile)
    {
        PhysicsBody body(profile.physicsBodyData);
        for (glm::vec2 feet : path)
        {
            body.setPosition(feet - body.bottomCenterOffset());
            for (glm::ivec2 touched : tileMap.tilesTouching(body.touchBox()))
                if (tileMap.getTileAtTilePosition(touched).isDeadly())
                    return true;
        }

        return false;
    }

    std::vector<ChosenFall> addFallLandingNodes(
        NavigationGraph &navigationGraph,
        const TileMap &tileMap,
        const NavigationProfile &profile,
        int headroom)
    {
        std::vector<ChosenFall> falls;
        if (!profile.falls())
            return falls;

        float stepHeight = profile.physicsBodyData.stepHeight;
        int nextNodeId = 0;
        std::vector<int> takeOffs;
        for (const auto &[id, node] : navigationGraph.getNodes())
        {
            nextNodeId = std::max(nextNodeId, id + 1);
            takeOffs.push_back(id);
        }

        while (!takeOffs.empty())
        {
            std::unordered_map<int, int> runs =
                runOfEachNode(navigationGraph, tileMap, headroom, stepHeight);
            std::vector<int> landedOn;
            for (int takeOffId : takeOffs)
                for (const JumpAttempt &fall :
                     fallsFrom(navigationGraph, tileMap, runs, takeOffId, profile, headroom))
                {
                    falls.push_back({takeOffId, fall.path, fall.inputs});
                    if (navigationGraph.hasNodeAtPosition(fall.path.back()))
                        continue;

                    navigationGraph.addNode(nextNodeId, fall.path.back(), NodeKind::Landing);
                    landedOn.push_back(nextNodeId++);
                }
            takeOffs = landedOn;
        }

        return falls;
    }

    void addFallEdges(
        NavigationGraph &navigationGraph,
        const TileMap &tileMap,
        const NavigationProfile &profile,
        int headroom,
        const std::vector<ChosenFall> &falls)
    {
        float stepHeight = profile.physicsBodyData.stepHeight;
        std::unordered_map<int, int> runs =
            runOfEachNode(navigationGraph, tileMap, headroom, stepHeight);

        for (const ChosenFall &fall : falls)
        {
            int fromId = fall.fromId;
            std::optional<int> toId =
                nodeGoverning(navigationGraph, tileMap, fall.path.back(), headroom, stepHeight);
            if (!toId || *toId == fromId || !runs.contains(*toId))
                continue;

            if (!landsFromAnywhereItTakesOff(
                    navigationGraph,
                    tileMap,
                    profile,
                    headroom,
                    fall.path,
                    fall.inputs,
                    runs,
                    runs.at(*toId)))
                continue;

            navigationGraph.addEdge(timed(
                {fromId, *toId, EdgeType::Fall, fall.path, fall.inputs}, navigationGraph, profile));
        }
    }
}
