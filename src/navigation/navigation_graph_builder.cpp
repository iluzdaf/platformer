#include <algorithm>
#include <cstddef>
#include <optional>
#include <map>
#include <utility>
#include <cmath>
#include <unordered_map>
#include <vector>
#include "navigation/navigation_graph_builder.hpp"
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
    constexpr float NodeTileNudge = 0.5f;

    constexpr float SurfaceTolerance = 1.0f;

    int tilesOfHeadroom(const TileMap &tileMap, const NavigationProfile &profile)
    {
        float tileSize = static_cast<float>(tileMap.getTileSize());
        return static_cast<int>(std::ceil(profile.colliderSize.y / tileSize));
    }

    bool canStandOn(const TileMap &tileMap, glm::ivec2 groundTilePosition, int headroom)
    {
        for (int offset = 1; offset <= headroom; ++offset)
        {
            glm::ivec2 above = groundTilePosition + glm::ivec2(0, -offset);
            if (!tileMap.validTilePosition(above))
                return false;

            const Tile &tile = tileMap.getTileAtTilePosition(above);
            if (tile.isSolid() || tile.isDeadly())
                return false;
        }

        return true;
    }

    bool isWalkableBetween(const TileMap &tileMap, glm::vec2 start, glm::vec2 end, int headroom)
    {
        if (start == end)
            return false;

        glm::vec2 underfoot(0.0f, 1.0f);
        glm::vec2 inwards = glm::normalize(end - start) * NodeTileNudge;
        glm::ivec2 startTilePosition = tileMap.worldToTilePosition(start + inwards + underfoot);
        glm::ivec2 endTilePosition = tileMap.worldToTilePosition(end - inwards + underfoot);

        if (startTilePosition.y != endTilePosition.y)
            return false;

        int fromX = std::min(startTilePosition.x, endTilePosition.x);
        int toX = std::max(startTilePosition.x, endTilePosition.x);

        for (int x = fromX; x <= toX; ++x)
        {
            glm::ivec2 groundTilePosition(x, startTilePosition.y);
            if (!tileMap.validTilePosition(groundTilePosition) ||
                !tileMap.getTileAtTilePosition(groundTilePosition).isSolid())
                return false;

            if (!canStandOn(tileMap, groundTilePosition, headroom))
                return false;
        }

        return true;
    }

    void addNodes(NavigationGraph &navigationGraph, const TileMap &tileMap, int headroom)
    {
        int nextNodeId = 0;

        for (int y = 0; y < tileMap.getHeight(); ++y)
        {
            for (int x = 0; x < tileMap.getWidth(); ++x)
            {
                glm::ivec2 tilePosition(x, y);
                if (!tileMap.getTileAtTilePosition(tilePosition).isSolid())
                    continue;

                bool canWalkAbove = canStandOn(tileMap, tilePosition, headroom);
                if (!canWalkAbove)
                    continue;

                glm::ivec2 tilePositionLeft = tilePosition + glm::ivec2(-1, 0);
                bool isLeftCliff = tileMap.validTilePosition(tilePositionLeft) &&
                                   !tileMap.getTileAtTilePosition(tilePositionLeft).isSolid();
                glm::ivec2 tilePositionRight = tilePosition + glm::ivec2(1, 0);
                bool isRightCliff = tileMap.validTilePosition(tilePositionRight) &&
                                    !tileMap.getTileAtTilePosition(tilePositionRight).isSolid();

                bool canWalkAboveLeft =
                    !isLeftCliff && canStandOn(tileMap, tilePositionLeft, headroom);
                bool canWalkAboveRight =
                    !isRightCliff && canStandOn(tileMap, tilePositionRight, headroom);

                if (!isLeftCliff && !isRightCliff && canWalkAboveLeft && canWalkAboveRight)
                    continue;

                glm::vec2 worldPosition = tileMap.tileToWorldPosition(tilePosition);
                float tileSize = static_cast<float>(tileMap.getTileSize());
                glm::vec2 nodeOffset(tileSize / 2.0f, 0.0f);

                if (canWalkAboveLeft && !canWalkAboveRight)
                    nodeOffset = glm::vec2(tileSize, 0.0f);
                else if (!canWalkAboveLeft && canWalkAboveRight)
                    nodeOffset = glm::vec2(0.0f, 0.0f);

                navigationGraph.addNode(nextNodeId++, worldPosition + nodeOffset);
            }
        }
    }

    void addWalkEdges(NavigationGraph &navigationGraph, const TileMap &tileMap, int headroom)
    {
        std::unordered_map<int, std::vector<NavigationNode>> nodesByRow;

        for (const auto &[id, node] : navigationGraph.getNodes())
            nodesByRow[static_cast<int>(std::round(node.position.y))].push_back(node);

        std::vector<int> rows;
        for (const auto &[y, nodesInRow] : nodesByRow)
            rows.push_back(y);
        std::sort(rows.begin(), rows.end());

        for (int y : rows)
        {
            std::vector<NavigationNode> &nodesInRow = nodesByRow[y];
            std::sort(
                nodesInRow.begin(),
                nodesInRow.end(),
                [](const NavigationNode &left, const NavigationNode &right)
                { return left.position.x < right.position.x; });

            for (size_t index = 1; index < nodesInRow.size(); ++index)
            {
                const NavigationNode &left = nodesInRow[index - 1];
                const NavigationNode &right = nodesInRow[index];

                if (!isWalkableBetween(tileMap, left.position, right.position, headroom))
                    continue;

                navigationGraph.addEdge(left.id, right.id, EdgeType::Walk);
                navigationGraph.addEdge(right.id, left.id, EdgeType::Walk);
            }
        }
    }

    bool clearAt(const TileMap &tileMap, glm::vec2 feetPosition, const NavigationProfile &profile)
    {
        constexpr float Inset = 0.5f;
        float halfWidth = profile.colliderSize.x * 0.5f - Inset;
        glm::vec2 low(feetPosition.x - halfWidth, feetPosition.y - profile.colliderSize.y + Inset);
        glm::vec2 high(feetPosition.x + halfWidth, feetPosition.y - Inset);

        glm::ivec2 lowTilePosition = tileMap.worldToTilePosition(low);
        glm::ivec2 highTilePosition = tileMap.worldToTilePosition(high);

        for (int y = lowTilePosition.y; y <= highTilePosition.y; ++y)
            for (int x = lowTilePosition.x; x <= highTilePosition.x; ++x)
            {
                glm::ivec2 tilePosition(x, y);
                if (!tileMap.validTilePosition(tilePosition))
                    return false;

                const Tile &tile = tileMap.getTileAtTilePosition(tilePosition);
                if (tile.isSolid() || tile.isDeadly())
                    return false;
            }

        return true;
    }

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

            if (!clearAt(tileMap, position, profile))
                return std::nullopt;
        }

        return std::nullopt;
    }

    std::optional<int> nodeGoverning(
        const NavigationGraph &navigationGraph,
        const TileMap &tileMap,
        glm::vec2 landing,
        int headroom)
    {
        constexpr float SameSurface = 0.5f;
        std::optional<int> nearest;
        float nearestDistance = 0.0f;

        for (const auto &[id, node] : navigationGraph.getNodes())
        {
            if (std::abs(node.position.y - landing.y) > SameSurface)
                continue;

            float distance = std::abs(node.position.x - landing.x);
            if (distance > SameSurface &&
                !isWalkableBetween(tileMap, landing, node.position, headroom))
                continue;

            if (nearest && distance >= nearestDistance)
                continue;

            nearest = id;
            nearestDistance = distance;
        }

        return nearest;
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

    bool alreadyConnected(const NavigationGraph &navigationGraph, int fromId, int toId)
    {
        for (const auto &edge : navigationGraph.getOutgoingEdges(fromId))
            if (edge.toId == toId)
                return true;

        return false;
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

    struct ChosenJump
    {
        int fromId = 0;
        std::vector<glm::vec2> path;
        float holdDuration = 0.0f;
    };

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

    std::optional<glm::vec2> surfaceBelow(
        const TileMap &tileMap,
        glm::vec2 from,
        const NavigationProfile &profile,
        int headroom)
    {
        float tileSize = static_cast<float>(tileMap.getTileSize());
        glm::ivec2 startTilePosition = tileMap.worldToTilePosition(from);

        for (int y = startTilePosition.y; y < tileMap.getHeight(); ++y)
        {
            glm::ivec2 groundTilePosition(startTilePosition.x, y);
            if (!tileMap.validTilePosition(groundTilePosition))
                return std::nullopt;

            glm::vec2 standing(from.x, static_cast<float>(y) * tileSize);

            if (tileMap.getTileAtTilePosition(groundTilePosition).isSolid())
            {
                if (y == startTilePosition.y)
                    return std::nullopt;

                return canStandOn(tileMap, groundTilePosition, headroom) &&
                               clearAt(tileMap, standing, profile)
                           ? std::optional(standing)
                           : std::nullopt;
            }

            if (!clearAt(tileMap, standing, profile))
                return std::nullopt;
        }

        return std::nullopt;
    }

    std::vector<glm::vec2> fallLandings(
        const TileMap &tileMap,
        glm::vec2 takeOff,
        const NavigationProfile &profile,
        int headroom)
    {
        float stride = profile.colliderSize.x * 0.5f + 1.0f;

        std::vector<glm::vec2> landings;
        for (float direction : {-1.0f, 1.0f})
        {
            glm::vec2 steppedOff(takeOff.x + direction * stride, takeOff.y);
            if (std::optional<glm::vec2> landing =
                    surfaceBelow(tileMap, steppedOff, profile, headroom))
                landings.push_back(*landing);
        }

        return landings;
    }

    void addFallLandingNodes(
        NavigationGraph &navigationGraph,
        const TileMap &tileMap,
        const NavigationProfile &profile,
        int headroom)
    {
        if (!profile.falls())
            return;

        int nextNodeId = 0;
        for (const auto &[id, node] : navigationGraph.getNodes())
            nextNodeId = std::max(nextNodeId, id + 1);

        for (bool added = true; added;)
        {
            added = false;
            std::vector<glm::vec2> takeOffs;
            for (const auto &[id, node] : navigationGraph.getNodes())
                takeOffs.push_back(node.position);

            for (glm::vec2 takeOff : takeOffs)
                for (glm::vec2 landing : fallLandings(tileMap, takeOff, profile, headroom))
                {
                    if (navigationGraph.hasNodeAtPosition(landing))
                        continue;

                    navigationGraph.addNode(nextNodeId++, landing, NodeKind::Landing);
                    added = true;
                }
        }
    }

    std::optional<glm::vec2> standingBelow(
        const TileMap &tileMap,
        float x,
        float below,
        const NavigationProfile &profile,
        int headroom)
    {
        float tileSize = static_cast<float>(tileMap.getTileSize());
        glm::ivec2 column = tileMap.worldToTilePosition(glm::vec2(x, below));

        for (int y = column.y + 1; y < tileMap.getHeight(); ++y)
        {
            glm::ivec2 ground(column.x, y);
            if (!tileMap.validTilePosition(ground))
                return std::nullopt;
            if (!tileMap.getTileAtTilePosition(ground).isSolid())
                continue;

            glm::vec2 standing(x, static_cast<float>(y) * tileSize);
            if (canStandOn(tileMap, ground, headroom) && clearAt(tileMap, standing, profile))
                return standing;

            return std::nullopt;
        }

        return std::nullopt;
    }

    void addTakeOffNodes(
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
                    tileMap.worldToTilePosition(attempt.path.back() + glm::vec2(0.0f, 1.0f));
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

            for (float x = ledge.x - reach; x <= ledge.x + reach; x += step)
            {
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

    bool canHangAt(const TileMap &tileMap, int climbX, int wallX, int footRow, int headroom)
    {
        for (int offset = 1; offset <= headroom; ++offset)
        {
            glm::ivec2 beside(wallX, footRow - offset);
            glm::ivec2 body(climbX, footRow - offset);
            if (!tileMap.validTilePosition(beside) || !tileMap.validTilePosition(body))
                return false;

            if (!tileMap.getTileAtTilePosition(beside).isSolid())
                return false;

            const Tile &bodyTile = tileMap.getTileAtTilePosition(body);
            if (bodyTile.isSolid() || bodyTile.isDeadly())
                return false;
        }

        return true;
    }

    glm::vec2 againstTheWall(const TileMap &tileMap, int climbX, int wallX, int footRow)
    {
        float tileSize = static_cast<float>(tileMap.getTileSize());
        glm::vec2 corner = tileMap.tileToWorldPosition(glm::ivec2(climbX, footRow));
        return corner + glm::vec2(wallX > climbX ? tileSize : 0.0f, 0.0f);
    }

    std::optional<int> nodeAtPosition(const NavigationGraph &navigationGraph, glm::vec2 position)
    {
        for (const auto &[id, node] : navigationGraph.getNodes())
            if (std::abs(node.position.x - position.x) <= 0.1f &&
                std::abs(node.position.y - position.y) <= 0.1f)
                return id;

        return std::nullopt;
    }

    std::optional<int> ledgeAboveTheFace(
        const NavigationGraph &navigationGraph,
        const TileMap &tileMap,
        int climbX,
        int wallX,
        int runTop,
        int headroom)
    {
        glm::ivec2 wallTop(wallX, runTop - headroom);
        glm::ivec2 above = wallTop + glm::ivec2(0, -1);
        if (tileMap.validTilePosition(above) && tileMap.getTileAtTilePosition(above).isSolid())
            return std::nullopt;

        return nodeGoverning(
            navigationGraph, tileMap, againstTheWall(tileMap, wallX, climbX, wallTop.y), headroom);
    }

    void addClimbing(
        NavigationGraph &navigationGraph,
        const TileMap &tileMap,
        const NavigationProfile &profile,
        int headroom)
    {
        if (!profile.climbs())
            return;

        int nextNodeId = 0;
        for (const auto &[id, node] : navigationGraph.getNodes())
            nextNodeId = std::max(nextNodeId, id + 1);

        auto joinBothWays = [&](int fromId, int toId, float wallDirection)
        {
            if (fromId == toId)
                return;

            navigationGraph.addEdge(
                NavigationEdge{fromId, toId, EdgeType::Climb, {}, 0.0f, wallDirection});
            navigationGraph.addEdge(
                NavigationEdge{toId, fromId, EdgeType::Climb, {}, 0.0f, wallDirection});
        };

        auto endOfTheFace = [&](int climbX, int wallX, int footRow)
        {
            glm::vec2 position = againstTheWall(tileMap, climbX, wallX, footRow);
            std::optional<int> existing = nodeAtPosition(navigationGraph, position);
            if (existing)
                return *existing;

            int id = nextNodeId++;
            navigationGraph.addNode(id, position, NodeKind::OnWall);
            return id;
        };

        for (int climbX = 0; climbX < tileMap.getWidth(); ++climbX)
            for (int side : {-1, 1})
            {
                int wallX = climbX + side;
                std::optional<int> runTop;

                for (int footRow = 0; footRow <= tileMap.getHeight(); ++footRow)
                {
                    if (footRow < tileMap.getHeight() &&
                        canHangAt(tileMap, climbX, wallX, footRow, headroom))
                    {
                        if (!runTop)
                            runTop = footRow;
                        continue;
                    }

                    if (!runTop)
                        continue;

                    int runBottom = footRow - 1;
                    std::optional<int> ledgeId = ledgeAboveTheFace(
                        navigationGraph, tileMap, climbX, wallX, *runTop, headroom);

                    if (runBottom == *runTop && !ledgeId)
                    {
                        runTop.reset();
                        continue;
                    }

                    int topId = endOfTheFace(climbX, wallX, *runTop);
                    float wallDirection = static_cast<float>(side);
                    joinBothWays(topId, endOfTheFace(climbX, wallX, runBottom), wallDirection);
                    if (ledgeId)
                        joinBothWays(topId, *ledgeId, wallDirection);

                    runTop.reset();
                }
            }
    }

    void addFallEdges(
        NavigationGraph &navigationGraph,
        const TileMap &tileMap,
        const NavigationProfile &profile,
        int headroom)
    {
        if (!profile.falls())
            return;

        std::vector<std::pair<int, glm::vec2>> takeOffs;
        for (const auto &[id, node] : navigationGraph.getNodes())
            takeOffs.emplace_back(id, node.position);

        for (const auto &[fromId, takeOff] : takeOffs)
            for (glm::vec2 landing : fallLandings(tileMap, takeOff, profile, headroom))
            {
                std::optional<int> toId =
                    nodeGoverning(navigationGraph, tileMap, landing, headroom);
                if (!toId || *toId == fromId)
                    continue;

                if (alreadyConnected(navigationGraph, fromId, *toId))
                    continue;

                navigationGraph.addEdge({fromId, *toId, EdgeType::Fall, {}, 0.0f});
            }
    }
}

NavigationGraph buildNavigationGraph(const TileMap &tileMap, const NavigationProfile &profile)
{
    NavigationGraph navigationGraph;
    int headroom = tilesOfHeadroom(tileMap, profile);

    addNodes(navigationGraph, tileMap, headroom);
    addFallLandingNodes(navigationGraph, tileMap, profile, headroom);
    addTakeOffNodes(navigationGraph, tileMap, profile, headroom);
    addWalkEdges(navigationGraph, tileMap, headroom);

    std::vector<ChosenJump> jumps = chooseJumps(navigationGraph, tileMap, profile, headroom);
    addJumpLandingNodes(navigationGraph, jumps);

    navigationGraph.clearEdges();
    addWalkEdges(navigationGraph, tileMap, headroom);

    addJumpEdges(navigationGraph, tileMap, headroom, jumps);
    addFallEdges(navigationGraph, tileMap, profile, headroom);

    addClimbing(navigationGraph, tileMap, profile, headroom);

    return navigationGraph;
}
