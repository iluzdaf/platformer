#include <algorithm>
#include <cstddef>
#include <optional>
#include <map>
#include <utility>
#include <cmath>
#include <unordered_map>
#include <vector>
#include "navigation/navigation_graph_steps.hpp"
#include "navigation/jump_arc.hpp"
#include "navigation/jump_simulation.hpp"
#include "navigation/navigation_profile.hpp"
#include "tile_map/tile.hpp"
#include "navigation/navigation_graph.hpp"
#include "navigation/navigation_node.hpp"
#include "navigation/navigation_edge.hpp"
#include "tile_map/tile_map.hpp"

namespace
{
    struct JumpLanding
    {
        glm::vec2 position;
        std::vector<glm::vec2> path;
    };

    std::optional<JumpLanding> landingOf(
        const TileMap &tileMap,
        glm::vec2 takeOff,
        const std::vector<glm::vec2> &arc,
        float direction,
        const NavigationProfile &profile)
    {
        float previousY = takeOff.y;
        std::vector<glm::vec2> path{takeOff};

        for (size_t index = 1; index < arc.size(); ++index)
        {
            glm::vec2 position = takeOff + glm::vec2(direction * arc[index].x, arc[index].y);
            bool descending = position.y >= previousY;
            previousY = position.y;
            path.push_back(position);

            if (descending)
            {
                glm::ivec2 underfoot =
                    tileMap.worldToTilePosition(position + glm::vec2(0.0f, 1.0f));
                if (tileMap.validTilePosition(underfoot) &&
                    tileMap.getTileAtTilePosition(underfoot).isSolid())
                {
                    glm::vec2 landing(
                        position.x, static_cast<float>(underfoot.y * tileMap.getTileSize()));
                    path.back() = landing;
                    return JumpLanding{landing, path};
                }
            }

            if (!navigation::clearAt(tileMap, position, profile))
                return std::nullopt;
        }

        return std::nullopt;
    }

    std::unordered_map<int, int> walkComponents(const NavigationGraph &navigationGraph)
    {
        std::unordered_map<int, int> components;

        int nextComponent = 0;
        for (const auto &[id, node] : navigationGraph.getNodes())
        {
            if (components.contains(id))
                continue;

            components[id] = nextComponent;

            std::vector<int> pending{id};
            while (!pending.empty())
            {
                int at = pending.back();
                pending.pop_back();

                for (const auto &edge : navigationGraph.getOutgoingEdges(at))
                {
                    if (edge.type != EdgeType::Walk || components.contains(edge.toId))
                        continue;

                    components[edge.toId] = nextComponent;
                    pending.push_back(edge.toId);
                }
            }

            ++nextComponent;
        }

        return components;
    }

    std::optional<JumpLanding> jumpFrom(
        const TileMap &tileMap,
        glm::vec2 takeOff,
        const JumpArc &arc,
        float direction,
        const NavigationProfile &profile)
    {
        if (!profile.motionData || !profile.physicsBodyData)
            return landingOf(tileMap, takeOff, arc.offsets, direction, profile);

        JumpAttempt attempt = simulateJumpAgainst(
            tileMap,
            *profile.motionData,
            *profile.physicsBodyData,
            takeOff,
            direction,
            arc.holdFraction);
        if (!attempt.landed)
            return std::nullopt;

        return JumpLanding{attempt.path.back(), attempt.path};
    }

    struct JumpCandidate
    {
        int toId = 0;
        std::vector<glm::vec2> path;
        float holdDuration = 0.0f;
        float reach = 0.0f;
    };

    bool easierThan(const JumpCandidate &candidate, const JumpCandidate &against)
    {
        if (candidate.holdDuration != against.holdDuration)
            return candidate.holdDuration < against.holdDuration;

        return candidate.reach < against.reach;
    }
}

namespace navigation
{
    std::vector<ChosenJump> chooseJumps(
        NavigationGraph &navigationGraph,
        const TileMap &tileMap,
        const NavigationProfile &profile,
        int headroom)
    {
        std::unordered_map<int, int> components = walkComponents(navigationGraph);

        std::map<std::pair<int, int>, JumpCandidate> easiest;

        std::vector<std::pair<int, glm::vec2>> takeOffs;
        for (const auto &[id, node] : navigationGraph.getNodes())
            if (node.kind == NodeKind::OnFoot)
                takeOffs.emplace_back(id, node.position);

        for (const auto &[fromId, takeOff] : takeOffs)
            for (const JumpArc &arc : profile.jumpArcs)
                for (float direction : {1.0f, -1.0f})
                {
                    std::optional<JumpLanding> landing =
                        jumpFrom(tileMap, takeOff, arc, direction, profile);
                    if (!landing || landing->position.y > takeOff.y)
                        continue;

                    std::optional<int> toId =
                        nodeGoverning(navigationGraph, tileMap, landing->position, headroom);
                    if (!toId || *toId == fromId)
                        continue;

                    if (components.at(fromId) == components.at(*toId))
                        continue;

                    float reach = std::abs(navigationGraph.getNode(*toId).position.x - takeOff.x);

                    JumpCandidate candidate{*toId, landing->path, arc.holdDuration, reach};

                    std::pair<int, int> platform(fromId, components.at(*toId));
                    auto found = easiest.find(platform);
                    if (found != easiest.end() && !easierThan(candidate, found->second))
                        continue;

                    easiest[platform] = candidate;
                }

        std::vector<ChosenJump> chosen;
        for (const auto &[platform, candidate] : easiest)
            chosen.push_back({platform.first, candidate.path, candidate.holdDuration});

        return chosen;
    }

    void addJumpLandingNodes(NavigationGraph &navigationGraph, const std::vector<ChosenJump> &jumps)
    {
        int nextNodeId = 0;
        for (const auto &[id, node] : navigationGraph.getNodes())
            nextNodeId = std::max(nextNodeId, id + 1);

        for (const ChosenJump &jump : jumps)
        {
            glm::vec2 comesDown = jump.path.back();

            if (!navigationGraph.hasNodeAtPosition(comesDown))
                navigationGraph.addNode(nextNodeId++, comesDown, NodeKind::Landing);
        }
    }

    void addJumpEdges(
        NavigationGraph &navigationGraph,
        const TileMap &tileMap,
        int headroom,
        const std::vector<ChosenJump> &jumps)
    {
        for (const ChosenJump &jump : jumps)
        {
            std::optional<int> toId =
                nodeGoverning(navigationGraph, tileMap, jump.path.back(), headroom);
            if (!toId || *toId == jump.fromId)
                continue;

            navigationGraph.addEdge(
                {jump.fromId, *toId, EdgeType::Jump, jump.path, jump.holdDuration});
        }
    }
}
