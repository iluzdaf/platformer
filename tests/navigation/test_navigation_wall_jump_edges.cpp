#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <optional>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "helpers/actors.hpp"
#include "helpers/graph_queries.hpp"
#include "helpers/palettes.hpp"
#include "helpers/tiles.hpp"
#include "helpers/navigation_maps.hpp"
#include "helpers/tile_positions.hpp"
#include "navigation/footing.hpp"
#include "navigation/jump_simulation.hpp"
#include "navigation/navigation_graph_steps.hpp"
#include "navigation/navigation_edge.hpp"
#include "navigation/navigation_graph.hpp"
#include "navigation/navigation_graph_builder.hpp"
#include "navigation/navigation_node.hpp"
#include "navigation/navigation_path.hpp"
#include "navigation/navigation_profile.hpp"
#include "tile_map/tile_collider_data.hpp"
#include "tile_map/tile_map.hpp"
#include "tile_map/tile_palette_data.hpp"
#include "timing/fixed_time_step.hpp"

namespace
{
    std::vector<NavigationEdge> leapsOffWalls(const NavigationGraph &graph)
    {
        std::vector<NavigationEdge> leaps;
        for (const NavigationEdge &edge : graph.getEdges())
            if (edge.type == EdgeType::Jump && graph.getNode(edge.fromId).kind == NodeKind::OnWall)
                leaps.push_back(edge);

        return leaps;
    }

    std::optional<NavigationEdge> leapOnto(const NavigationGraph &graph, float surface)
    {
        for (const NavigationEdge &edge : leapsOffWalls(graph))
            if (graph.getNode(edge.toId).feet.y == surface)
                return edge;

        return std::nullopt;
    }

    bool reachesTheShelf(const NavigationGraph &graph)
    {
        for (const auto &[fromId, from] : graph.getNodes())
            for (const auto &[toId, to] : graph.getNodes())
                if (from.feet.y == surfaceOf(ShelfFloorRow) && to.feet.y == surfaceOf(ShelfRow) &&
                    !findPath(graph, fromId, toId).empty())
                    return true;

        return false;
    }

    NavigationProfile withoutAWallJump(NavigationProfile profile)
    {
        profile.abilities.wallJump.reset();
        return profile;
    }
}

TEST_CASE(
    "A climber that can wall jump leaps from its wall to a shelf nothing else reaches",
    "[NavigationGraphBuilder][WallJump]")
{
    TileMap tileMap = aWallAcrossFromAShelfMap();

    NavigationGraph leaping = buildNavigationGraph(tileMap, wallJumperProfile());
    NavigationGraph climbing = buildNavigationGraph(tileMap, withoutAWallJump(wallJumperProfile()));

    REQUIRE(leapOnto(leaping, surfaceOf(ShelfRow)));
    REQUIRE(reachesTheShelf(leaping));
    REQUIRE_FALSE(reachesTheShelf(climbing));
    REQUIRE(leapsOffWalls(climbing).empty());
}

TEST_CASE(
    "A leap leaves from a hold partway up its wall that climbs join, and says which side the "
    "wall is on",
    "[NavigationGraphBuilder][WallJump]")
{
    TileMap tileMap = aWallAcrossFromAShelfMap();
    NavigationGraph graph = buildNavigationGraph(tileMap, wallJumperProfile());

    std::optional<NavigationEdge> leap = leapOnto(graph, surfaceOf(ShelfRow));
    REQUIRE(leap);
    NavigationNode takeOff = graph.getNode(leap->fromId);
    float faceX = topLeftOf({ShelfWallX + 1, 0}).x;

    REQUIRE(takeOff.kind == NodeKind::OnWall);
    REQUIRE(takeOff.feet.x == faceX);
    REQUIRE(takeOff.feet.y > surfaceOf(ShelfWallTopRow + 1));
    REQUIRE(takeOff.feet.y < surfaceOf(ShelfFloorRow));
    for (float y : {surfaceOf(ShelfWallTopRow + 1), surfaceOf(ShelfFloorRow)})
    {
        REQUIRE(hasEdgeBetween(graph, glm::vec2(faceX, y), takeOff.feet, EdgeType::Climb));
        REQUIRE(hasEdgeBetween(graph, takeOff.feet, glm::vec2(faceX, y), EdgeType::Climb));
    }
    REQUIRE(leap->wallDirection == -1.0f);
    REQUIRE(leap->inputs.front().pressed.jumpHeld);
    REQUIRE(leap->inputs.front().pressed.direction.x == 1.0f);
    REQUIRE(
        *leap->duration == Catch::Approx(static_cast<float>(leap->path.size() - 1) * PhysicsStep));
}

TEST_CASE(
    "Where a wall jump comes down, walks join it to the rest of the ground",
    "[NavigationGraphBuilder][WallJump]")
{
    TileMap tileMap = aWallAcrossFromAShelfMap();
    NavigationGraph graph = buildNavigationGraph(tileMap, wallJumperProfile());

    REQUIRE_FALSE(leapsOffWalls(graph).empty());
    for (const NavigationEdge &leap : leapsOffWalls(graph))
    {
        bool walksOn = false;
        bool walkedTo = false;
        for (const NavigationEdge &edge : graph.getEdges())
        {
            walksOn = walksOn || (edge.type == EdgeType::Walk && edge.fromId == leap.toId);
            walkedTo = walkedTo || (edge.type == EdgeType::Walk && edge.toId == leap.toId);
        }

        REQUIRE(walksOn);
        REQUIRE(walkedTo);
    }
}

namespace
{
    TileMap aWallAcrossFromAShelfAt(int row, int from, const TilePaletteData &palette)
    {
        Placed laid;
        layRow(laid, ShelfFloorRow, 0, ShelfSceneTiles - 1, SlipperyTile);
        layColumn(laid, ShelfWallX, ShelfWallTopRow, ShelfFloorRow - 1);
        layRow(laid, row, from, ShelfSceneTiles - 1, SlipperyTile);
        return aTileMap(laid, ShelfSceneTiles, ShelfSceneTiles, TestTileSize, palette);
    }

    NavigationProfile aWallJumperMovingAt(float moveSpeed)
    {
        NavigationProfile profile = wallJumperProfile();
        profile.abilities.move->moveSpeed = moveSpeed;
        return profile;
    }

    JumpAttempt steeredAllTheWayFromTheShelfWall(
        const TileMap &tileMap,
        const NavigationProfile &profile,
        float below = 0.0f)
    {
        glm::vec2 hold = topLeftOf({ShelfWallX + 1, ShelfWallTopRow + 1});
        JumpAttempt leap = simulateWallJumpAgainst(
            tileMap, profile.abilities, profile.physicsBodyData, hold, -1.0f);
        return simulateInputsAgainst(
            tileMap,
            profile.abilities,
            profile.physicsBodyData,
            hold + glm::vec2(0.0f, below),
            leap.inputs,
            leap.path.back().x,
            -1.0f);
    }
}

TEST_CASE(
    "A leap that lands from its hold but not from a pixel below it, where a climber may stop, "
    "is not offered",
    "[NavigationGraphBuilder][WallJump]")
{
    constexpr int ShelfAcrossRow = 9;
    constexpr int ShelfAcrossFrom = 12;
    TileMap tileMap =
        aWallAcrossFromAShelfAt(ShelfAcrossRow, ShelfAcrossFrom, aPaletteWithSlipperyTiles());
    NavigationProfile profile = aWallJumperMovingAt(181.4f);
    JumpAttempt fromTheHold = steeredAllTheWayFromTheShelfWall(tileMap, profile);
    JumpAttempt fromBelow = steeredAllTheWayFromTheShelfWall(tileMap, profile, ClimbArrivesWithin);
    REQUIRE(fromTheHold.path.back().y == surfaceOf(ShelfAcrossRow));
    REQUIRE(navigation::feetOverGround(tileMap, fromTheHold.path.back()));
    REQUIRE(fromBelow.path.back().y != surfaceOf(ShelfAcrossRow));

    NavigationGraph graph = buildNavigationGraph(tileMap, profile);

    REQUIRE_FALSE(leapOnto(graph, surfaceOf(ShelfAcrossRow)));
}

TEST_CASE(
    "A leap that comes down on the corner of a shelf, its feet past the edge of the collider, "
    "is not offered",
    "[NavigationGraphBuilder][WallJump]")
{
    constexpr int ShelfAcrossRow = 7;
    constexpr int CornerX = 10;
    constexpr int InsetTile = 4;
    TilePaletteData palette = aPaletteWithSlipperyTiles();
    palette.tiles[InsetTile] = palette.tiles[SlipperyTile];
    palette.tiles[InsetTile].collider =
        TileColliderData{glm::vec2(4.0f, 0.0f), glm::vec2(12.0f, 16.0f)};
    Placed laid;
    layRow(laid, ShelfFloorRow, 0, ShelfSceneTiles - 1, SlipperyTile);
    layColumn(laid, ShelfWallX, ShelfWallTopRow, ShelfFloorRow - 1);
    laid.push_back({glm::ivec2(CornerX, ShelfAcrossRow), InsetTile});
    layRow(laid, ShelfAcrossRow, CornerX + 1, ShelfSceneTiles - 1, SlipperyTile);
    TileMap tileMap = aTileMap(laid, ShelfSceneTiles, ShelfSceneTiles, TestTileSize, palette);
    NavigationProfile profile = aWallJumperMovingAt(157.0f);
    JumpAttempt fromTheHold = simulateWallJumpAgainst(
        tileMap,
        profile.abilities,
        profile.physicsBodyData,
        topLeftOf({ShelfWallX + 1, ShelfWallTopRow + 1}),
        -1.0f);
    INFO("lands " << fromTheHold.path.back().x << "," << fromTheHold.path.back().y);
    REQUIRE(tileMap.tileContaining(fromTheHold.path.back()).x == CornerX);
    REQUIRE_FALSE(navigation::feetOverGround(tileMap, fromTheHold.path.back()));
    for (float below : {-ClimbArrivesWithin, 0.0f, ClimbArrivesWithin})
        REQUIRE(
            steeredAllTheWayFromTheShelfWall(tileMap, profile, below).path.back().y ==
            surfaceOf(ShelfAcrossRow));

    NavigationGraph graph = buildNavigationGraph(tileMap, profile);

    REQUIRE_FALSE(leapOnto(graph, surfaceOf(ShelfAcrossRow)));
}

TEST_CASE("A leap through anything deadly is not offered", "[NavigationGraphBuilder][WallJump]")
{
    Placed laid = aWallAcrossFromAShelf();
    laid.push_back({glm::ivec2(9, 4), SpikeTile});
    TilePaletteData palette = aPaletteWithSlipperyTiles();
    palette.tiles[SpikeTile].deadly = true;
    TileMap tileMap = aTileMap(laid, ShelfSceneTiles, ShelfSceneTiles, TestTileSize, palette);
    NavigationProfile profile = wallJumperProfile();
    REQUIRE(
        navigation::touchesDeadly(
            tileMap, steeredAllTheWayFromTheShelfWall(tileMap, profile).path, profile));

    NavigationGraph graph = buildNavigationGraph(tileMap, profile);

    REQUIRE_FALSE(leapsOffWalls(graph).empty());
    for (const NavigationEdge &leap : leapsOffWalls(graph))
        REQUIRE_FALSE(navigation::touchesDeadly(tileMap, leap.path, profile));
}

TEST_CASE(
    "A body a hair from spikes touches them as the game counts a touch, and one over the bare "
    "part of a spike tile does not",
    "[NavigationGraphBuilder][WallJump]")
{
    constexpr glm::ivec2 Spikes{5, 5};
    TilePaletteData palette = aPaletteWithASolidTile();
    palette.tiles[SpikeTile].deadly = true;
    palette.tiles[SpikeTile].collider =
        TileColliderData{glm::vec2(0.0f, 12.0f), glm::vec2(16.0f, 4.0f)};
    TileMap tileMap = aTileMap({{Spikes, SpikeTile}}, 10, 10, TestTileSize, palette);
    NavigationProfile profile = wallJumperProfile();
    float halfWidth = profile.physicsBodyData.colliderSize.x * 0.5f;
    glm::vec2 floorOfTheSpikes = topLeftOf(Spikes + glm::ivec2(0, 1));

    glm::vec2 aHairAway = floorOfTheSpikes - glm::vec2(halfWidth + 0.05f, 0.0f);
    glm::vec2 wellAway = floorOfTheSpikes - glm::vec2(halfWidth + 0.5f, 0.0f);
    glm::vec2 aboveTheirPoints = floorOfTheSpikes + glm::vec2(halfWidth + 4.0f, -4.5f);

    REQUIRE(navigation::touchesDeadly(tileMap, {aHairAway}, profile));
    REQUIRE_FALSE(navigation::touchesDeadly(tileMap, {wellAway}, profile));
    REQUIRE_FALSE(navigation::touchesDeadly(tileMap, {aboveTheirPoints}, profile));
}
