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
    constexpr float SurfaceTolerance = 1.0f;

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
                glm::ivec2 underfoot = tileMap.tileContaining(position + glm::vec2(0.0f, 1.0f));
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

    std::optional<glm::vec2> standingBelow(
        const TileMap &tileMap,
        float x,
        float below,
        const NavigationProfile &profile,
        int headroom)
    {
        float tileSize = static_cast<float>(tileMap.getTileSize());
        glm::ivec2 column = tileMap.tileContaining(glm::vec2(x, below));

        for (int y = column.y + 1; y < tileMap.getHeight(); ++y)
        {
            glm::ivec2 ground(column.x, y);
            if (!tileMap.validTilePosition(ground))
                return std::nullopt;
            if (!tileMap.getTileAtTilePosition(ground).isSolid())
                continue;

            glm::vec2 standing(x, static_cast<float>(y) * tileSize);
            if (navigation::canStandOn(tileMap, ground, headroom) &&
                navigation::clearAt(tileMap, standing, profile))
                return standing;

            return std::nullopt;
        }

        return std::nullopt;
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
        std::unordered_map<int, int> components;
        std::vector<std::vector<int>> runs = walkRuns(navigationGraph, tileMap, headroom);
        for (size_t run = 0; run < runs.size(); ++run)
            for (int id : runs[run])
                components[id] = static_cast<int>(run);

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
        chosen.reserve(easiest.size());
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

    void addJumpTakeOffNodes(
        NavigationGraph &navigationGraph,
        const TileMap &tileMap,
        const NavigationProfile &profile,
        int headroom)
    {
        if (!profile.motionData || !profile.physicsBodyData || profile.jumpArcs.empty())
            return;

        float reach = 0.0f;
        for (const JumpArc &arc : profile.jumpArcs)
            for (glm::vec2 offset : arc.offsets)
                reach = std::max(reach, std::abs(offset.x));

        int nextNodeId = 0;
        std::vector<glm::vec2> ledges;
        std::vector<glm::vec2> couldJump;
        for (const auto &[id, node] : navigationGraph.getNodes())
        {
            nextNodeId = std::max(nextNodeId, id + 1);
            ledges.push_back(node.position);
            if (node.kind == NodeKind::OnFoot)
                couldJump.push_back(node.position);
        }

        auto landingOnTheLedge = [&](glm::vec2 from, glm::vec2 ledge) -> std::optional<glm::vec2>
        {
            float towards = ledge.x > from.x ? 1.0f : -1.0f;
            std::optional<glm::vec2> comesDown;
            float heldFor = 0.0f;

            for (const JumpArc &arc : profile.jumpArcs)
            {
                JumpAttempt attempt = simulateJumpAgainst(
                    tileMap,
                    *profile.motionData,
                    *profile.physicsBodyData,
                    from,
                    towards,
                    arc.holdFraction);
                if (!attempt.landed ||
                    std::abs(attempt.path.back().y - ledge.y) >= SurfaceTolerance)
                    continue;

                glm::ivec2 underfoot =
                    tileMap.tileContaining(attempt.path.back() + glm::vec2(0.0f, 1.0f));
                if (!tileMap.validTilePosition(underfoot) ||
                    !tileMap.getTileAtTilePosition(underfoot).isSolid() ||
                    !canStandOn(tileMap, underfoot, headroom))
                    continue;

                if (!comesDown || arc.holdDuration < heldFor)
                {
                    comesDown = attempt.path.back();
                    heldFor = arc.holdDuration;
                }
            }

            return comesDown;
        };

        float step = profile.colliderSize.x;
        for (glm::vec2 ledge : ledges)
        {
            bool alreadyServed = false;
            for (glm::vec2 standing : couldJump)
                if (standing.y > ledge.y && std::abs(standing.x - ledge.x) <= reach &&
                    landingOnTheLedge(standing, ledge))
                    alreadyServed = true;

            if (alreadyServed)
                continue;

            std::optional<glm::vec2> aimedAt;
            float overshoot = 0.0f;

            int steps = static_cast<int>(2.0f * reach / step);
            for (int taken = 0; taken <= steps; ++taken)
            {
                float x = ledge.x - reach + static_cast<float>(taken) * step;
                std::optional<glm::vec2> takeOff =
                    standingBelow(tileMap, x, ledge.y, profile, headroom);
                if (!takeOff)
                    continue;

                bool crowded = false;
                for (const auto &[id, node] : navigationGraph.getNodes())
                    if (glm::distance(node.position, *takeOff) < profile.colliderSize.x)
                        crowded = true;

                if (crowded)
                    continue;

                std::optional<glm::vec2> comesDown = landingOnTheLedge(*takeOff, ledge);
                if (!comesDown)
                    continue;

                float past = std::abs(comesDown->x - ledge.x);
                if (!aimedAt || past < overshoot)
                {
                    aimedAt = *takeOff;
                    overshoot = past;
                }
            }

            if (aimedAt)
                navigationGraph.addNode(nextNodeId++, *aimedAt);
        }
    }
}
