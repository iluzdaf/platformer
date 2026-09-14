#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <map>
#include <optional>
#include <set>
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
#include "navigation/navigation_path.hpp"
#include "navigation/navigation_profile.hpp"
#include "tile_map/tile_map.hpp"
#include "timing/fixed_time_step.hpp"

namespace
{
    constexpr float Never = std::numeric_limits<float>::infinity();
    constexpr float ReliablyQuicker = 0.15f;

    struct Entry
    {
        glm::vec2 feet;
        std::unordered_map<int, float> costs;
    };

    struct Pick
    {
        int row = 0;
        navigation::Leap leap;
        float seconds = 0.0f;
    };

    float secondsToWalk(const NavigationProfile &profile, float across)
    {
        if (profile.abilities.move && profile.abilities.move->moveSpeed > 0.0f)
            return std::abs(across) / profile.abilities.move->moveSpeed;

        return across == 0.0f ? 0.0f : Never;
    }

    float quickestWithoutLeaping(
        const NavigationGraph &navigationGraph,
        const NavigationProfile &profile,
        const std::unordered_map<int, int> &runs,
        const Entry &entry,
        int run,
        glm::vec2 landing)
    {
        float quickest = Never;
        for (const auto &[id, runOf] : runs)
        {
            auto reached = entry.costs.find(id);
            if (runOf != run || reached == entry.costs.end())
                continue;

            float across = landing.x - navigationGraph.getNode(id).feet.x;
            quickest = std::min(quickest, reached->second + secondsToWalk(profile, across));
        }

        return quickest;
    }

    bool enteredFromElsewhere(
        const NavigationGraph &navigationGraph,
        const navigation::ClimbFace &face,
        int endId)
    {
        for (const NavigationEdge &edge : navigationGraph.getEdges())
            if (edge.toId == endId && edge.fromId != face.topId && edge.fromId != face.bottomId)
                return true;

        return false;
    }

    bool roomToLeapFrom(
        const TileMap &tileMap,
        const navigation::ClimbFace &face,
        int row,
        int headroom)
    {
        glm::ivec2 hold(face.climbX, row);
        return !tileMap.groundAt(hold) && navigation::canStandOn(tileMap, hold, headroom + 1);
    }
}

namespace navigation
{
    std::vector<Leap> leapsFrom(
        NavigationGraph &navigationGraph,
        const TileMap &tileMap,
        const NavigationProfile &profile,
        int headroom,
        glm::vec2 hold,
        float wallDirection)
    {
        float stepHeight = profile.physicsBodyData.stepHeight;
        std::vector<Leap> leaps;
        auto keep = [&](JumpAttempt attempt)
        {
            navigationGraph.building().noting(attempt);
            if (!attempt.landed || !feetOverGround(tileMap, attempt.path.back()) ||
                touchesDeadly(tileMap, attempt.path, profile))
                return;

            std::optional<int> landsBy =
                nodeGoverning(navigationGraph, tileMap, attempt.path.back(), headroom, stepHeight);
            if (landsBy)
                leaps.push_back({std::move(attempt), *landsBy});
        };

        float away = -wallDirection;
        JumpAttempt steeredAllTheWay = simulateWallJumpAgainst(
            tileMap, profile.abilities, profile.physicsBodyData, hold, wallDirection);
        float reach = 0.0f;
        for (glm::vec2 feet : steeredAllTheWay.path)
            reach = std::max(reach, (feet.x - hold.x) * away);
        keep(steeredAllTheWay);

        for (const auto &[aimedAtId, aimedAt] : navigationGraph.getNodes())
        {
            float along = (aimedAt.feet.x - hold.x) * away;
            if (aimedAt.kind == NodeKind::OnWall || along <= 0.0f || along >= reach)
                continue;

            keep(simulateInputsAgainst(
                tileMap,
                profile.abilities,
                profile.physicsBodyData,
                hold,
                aWallJumpAwayFrom(wallDirection),
                aimedAt.feet.x,
                wallDirection));
        }

        return leaps;
    }

    void addWallJumps(
        NavigationGraph &navigationGraph,
        const TileMap &tileMap,
        const NavigationProfile &profile,
        int headroom,
        const std::vector<ClimbFace> &faces)
    {
        if (!profile.abilities.wallJump || !profile.abilities.wallClimb)
            return;

        float climbSpeed = profile.abilities.wallClimb->climbSpeed;
        float stepHeight = profile.physicsBodyData.stepHeight;
        std::unordered_map<int, int> runs =
            runOfEachNode(navigationGraph, tileMap, headroom, stepHeight);

        std::vector<std::pair<const ClimbFace *, Pick>> picked;
        for (const ClimbFace &face : faces)
        {
            float wallDirection = static_cast<float>(face.wallX - face.climbX);
            std::vector<Entry> entries;
            for (int id : std::set{face.topId, face.bottomId})
                if (enteredFromElsewhere(navigationGraph, face, id))
                    entries.push_back(
                        {navigationGraph.getNode(id).feet, costsFrom(navigationGraph, id)});
            if (entries.empty())
                continue;

            std::map<std::pair<std::size_t, int>, Pick> quickest;
            for (int row = face.topRow; row <= face.bottomRow; ++row)
            {
                if (!roomToLeapFrom(tileMap, face, row, headroom))
                    continue;

                glm::vec2 hold = againstTheWall(tileMap, face.climbX, face.wallX, row);
                for (Leap &leap :
                     leapsFrom(navigationGraph, tileMap, profile, headroom, hold, wallDirection))
                {
                    auto landsOn = runs.find(leap.landsBy);
                    if (landsOn == runs.end())
                        continue;

                    int run = landsOn->second;
                    float flight = static_cast<float>(leap.attempt.path.size() - 1) * PhysicsStep;
                    std::optional<bool> sound;
                    for (std::size_t from = 0; from < entries.size(); ++from)
                    {
                        float seconds =
                            std::abs(hold.y - entries[from].feet.y) / climbSpeed + flight;
                        if (seconds + ReliablyQuicker >= quickestWithoutLeaping(
                                                             navigationGraph,
                                                             profile,
                                                             runs,
                                                             entries[from],
                                                             run,
                                                             leap.attempt.path.back()))
                            continue;

                        auto found = quickest.find({from, run});
                        if (found != quickest.end() && found->second.seconds <= seconds)
                            continue;

                        if (!sound)
                            sound = landsFromAnywhereItTakesOff(
                                navigationGraph,
                                tileMap,
                                profile,
                                headroom,
                                leap.attempt.path,
                                leap.attempt.inputs,
                                runs,
                                run,
                                wallDirection);
                        if (*sound)
                            quickest[{from, run}] = {row, leap, seconds};
                    }
                }
            }

            for (const auto &[fromAndRun, pick] : quickest)
                picked.emplace_back(&face, pick);
        }

        int nextNodeId = 0;
        for (const auto &[id, node] : navigationGraph.getNodes())
            nextNodeId = std::max(nextNodeId, id + 1);

        auto nodeAt = [&](glm::vec2 feet, NodeKind kind)
        {
            if (std::optional<int> existing = navigationGraph.nodeAtPosition(feet))
                return std::pair(*existing, false);

            navigationGraph.addNode(nextNodeId, feet, kind);
            return std::pair(nextNodeId++, true);
        };

        std::map<const ClimbFace *, std::set<int>> holdsOnEachFace;
        for (const auto &[face, pick] : picked)
            holdsOnEachFace[face].insert(pick.row);

        std::map<std::pair<const ClimbFace *, int>, int> holdIds;
        for (const auto &[face, rows] : holdsOnEachFace)
        {
            float wallDirection = static_cast<float>(face->wallX - face->climbX);
            std::vector<int> upTheFace{face->topId};
            for (int row : rows)
            {
                int id =
                    nodeAt(
                        againstTheWall(tileMap, face->climbX, face->wallX, row), NodeKind::OnWall)
                        .first;
                holdIds[{face, row}] = id;
                if (id != upTheFace.back())
                    upTheFace.push_back(id);
            }
            if (face->bottomId != upTheFace.back())
                upTheFace.push_back(face->bottomId);

            for (std::size_t index = 1; index < upTheFace.size(); ++index)
                for (auto [fromId, toId] :
                     {std::pair(upTheFace[index - 1], upTheFace[index]),
                      std::pair(upTheFace[index], upTheFace[index - 1])})
                    navigationGraph.addEdge(timed(
                        {fromId, toId, EdgeType::Climb, {}, {}, wallDirection},
                        navigationGraph,
                        profile));
        }

        std::set<std::pair<int, int>> leapt;
        for (const auto &[face, pick] : picked)
        {
            int fromId = holdIds.at({face, pick.row});
            auto [toId, fresh] = nodeAt(pick.leap.attempt.path.back(), NodeKind::Landing);
            if (fresh)
                addWalksTo(navigationGraph, tileMap, profile, headroom, toId);
            if (!leapt.insert({fromId, toId}).second)
                continue;

            navigationGraph.addEdge(timed(
                {fromId,
                 toId,
                 EdgeType::Jump,
                 pick.leap.attempt.path,
                 pick.leap.attempt.inputs,
                 static_cast<float>(face->wallX - face->climbX)},
                navigationGraph,
                profile));
        }
    }
}
