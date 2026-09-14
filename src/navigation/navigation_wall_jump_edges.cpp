#include <algorithm>
#include <cstddef>
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
#include "navigation/navigation_profile.hpp"
#include "tile_map/tile_map.hpp"

namespace
{
    struct Pick
    {
        int row = 0;
        navigation::Leap leap;
    };

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
        if (!profile.abilities.wallJump)
            return;

        float stepHeight = profile.physicsBodyData.stepHeight;
        std::unordered_map<int, int> runs =
            runOfEachNode(navigationGraph, tileMap, headroom, stepHeight);

        std::vector<std::pair<const ClimbFace *, Pick>> picked;
        for (const ClimbFace &face : faces)
        {
            float wallDirection = static_cast<float>(face.wallX - face.climbX);
            for (int row = face.topRow; row <= face.bottomRow; ++row)
            {
                if (!roomToLeapFrom(tileMap, face, row, headroom))
                    continue;

                glm::vec2 hold = againstTheWall(tileMap, face.climbX, face.wallX, row);
                std::map<int, Leap> quickest;
                for (Leap &leap :
                     leapsFrom(navigationGraph, tileMap, profile, headroom, hold, wallDirection))
                {
                    auto landsOn = runs.find(leap.landsBy);
                    if (landsOn == runs.end())
                        continue;

                    auto found = quickest.find(landsOn->second);
                    if (found != quickest.end() &&
                        found->second.attempt.path.size() <= leap.attempt.path.size())
                        continue;

                    if (landsFromAnywhereItTakesOff(
                            navigationGraph,
                            tileMap,
                            profile,
                            headroom,
                            leap.attempt.path,
                            leap.attempt.inputs,
                            runs,
                            landsOn->second,
                            wallDirection))
                        quickest[landsOn->second] = leap;
                }

                for (const auto &[run, leap] : quickest)
                    picked.push_back({&face, {row, leap}});
            }
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
