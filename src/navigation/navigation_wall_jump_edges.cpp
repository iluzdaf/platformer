#include <algorithm>
#include <map>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "navigation/navigation_graph_steps.hpp"
#include "navigation/input_program.hpp"
#include "navigation/jump_simulation.hpp"
#include "navigation/navigation_build_report.hpp"
#include "navigation/navigation_edge.hpp"
#include "navigation/navigation_graph.hpp"
#include "navigation/navigation_node.hpp"
#include "navigation/navigation_profile.hpp"
#include "tile_map/tile_map.hpp"

namespace
{
    struct WallJump
    {
        int fromId = 0;
        float wallDirection = 0.0f;
        JumpAttempt attempt;
    };

    std::optional<float> wallHeldAt(const NavigationGraph &navigationGraph, int nodeId)
    {
        for (const NavigationEdge &edge : navigationGraph.getOutgoingEdges(nodeId))
            if (edge.type == EdgeType::Climb)
                return edge.wallDirection;

        return std::nullopt;
    }
}

namespace navigation
{
    void addWallJumps(
        NavigationGraph &navigationGraph,
        const TileMap &tileMap,
        const NavigationProfile &profile,
        int headroom)
    {
        if (!profile.abilities.wallJump)
            return;

        float stepHeight = profile.physicsBodyData.stepHeight;
        std::unordered_map<int, int> runs =
            runOfEachNode(navigationGraph, tileMap, headroom, stepHeight);

        std::map<std::pair<int, int>, WallJump> quickest;
        auto consider = [&](int fromId, float wallDirection, const JumpAttempt &attempt)
        {
            navigationGraph.building().noting(attempt);
            if (!attempt.landed || !feetOverGround(tileMap, attempt.path.back()) ||
                touchesDeadly(tileMap, attempt.path, profile))
                return;

            std::optional<int> toId =
                nodeGoverning(navigationGraph, tileMap, attempt.path.back(), headroom, stepHeight);
            if (!toId || !runs.contains(*toId))
                return;

            std::pair<int, int> leap(fromId, runs.at(*toId));
            auto found = quickest.find(leap);
            if (found != quickest.end() && found->second.attempt.path.size() <= attempt.path.size())
                return;

            if (landsFromAnywhereItTakesOff(
                    navigationGraph,
                    tileMap,
                    profile,
                    headroom,
                    attempt.path,
                    attempt.inputs,
                    runs,
                    leap.second,
                    wallDirection))
                quickest[leap] = {fromId, wallDirection, attempt};
        };

        for (const auto &[id, node] : navigationGraph.getNodes())
        {
            std::optional<float> wallDirection = wallHeldAt(navigationGraph, id);
            if (node.kind != NodeKind::OnWall || !wallDirection)
                continue;

            for (float direction : {-*wallDirection, *wallDirection})
            {
                JumpAttempt steeredAllTheWay = simulateWallJumpAgainst(
                    tileMap,
                    profile.abilities,
                    profile.physicsBodyData,
                    node.feet,
                    *wallDirection,
                    direction);
                consider(id, *wallDirection, steeredAllTheWay);

                float reach = 0.0f;
                for (glm::vec2 feet : steeredAllTheWay.path)
                    reach = std::max(reach, (feet.x - node.feet.x) * direction);

                for (const auto &[aimedAtId, aimedAt] : navigationGraph.getNodes())
                {
                    float along = (aimedAt.feet.x - node.feet.x) * direction;
                    if (aimedAt.kind == NodeKind::OnWall || along <= 0.0f || along >= reach)
                        continue;

                    consider(
                        id,
                        *wallDirection,
                        simulateInputsAgainst(
                            tileMap,
                            profile.abilities,
                            profile.physicsBodyData,
                            node.feet,
                            aWallJumpAwayFrom(*wallDirection),
                            aimedAt.feet.x,
                            *wallDirection));
                }
            }
        }

        int nextNodeId = 0;
        for (const auto &[id, node] : navigationGraph.getNodes())
            nextNodeId = std::max(nextNodeId, id + 1);

        for (const auto &[leap, jump] : quickest)
        {
            glm::vec2 landing = jump.attempt.path.back();
            std::optional<int> toId = navigationGraph.nodeAtPosition(landing);
            if (!toId)
            {
                toId = nextNodeId++;
                navigationGraph.addNode(*toId, landing, NodeKind::Landing);
                addWalksTo(navigationGraph, tileMap, profile, headroom, *toId);
            }

            navigationGraph.addEdge(timed(
                {jump.fromId,
                 *toId,
                 EdgeType::Jump,
                 jump.attempt.path,
                 jump.attempt.inputs,
                 jump.wallDirection},
                navigationGraph,
                profile));
        }
    }
}
