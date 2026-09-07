#pragma once

#include <string>
#include <utility>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/abilities/gravity_ability_data.hpp"
#include "actor/abilities/move_ability_data.hpp"
#include "actor/behaviors/patrol_behavior_data.hpp"
#include "actor/behaviors/state_machine_behavior_data.hpp"
#include "game/game_data.hpp"
#include "game/level.hpp"
#include "game/level_data.hpp"
#include "helpers/palettes.hpp"
#include "helpers/shipped.hpp"
#include "helpers/levels.hpp"
#include "helpers/tiles.hpp"
#include "npc/npc.hpp"
#include "npc/npc_data.hpp"
#include "npc/npc_spawn_data.hpp"
#include "tile_map/tile_data.hpp"
#include "tile_map/tile_map_data.hpp"
#include "tile_map/tile_palette_data.hpp"

inline constexpr glm::ivec2 SpawnTile{4, 5};

inline NpcData setupNpcData()
{
    NpcData npcData;
    npcData.actorData.size = glm::vec2(16.0f);
    npcData.actorData.motionData.moveAbilityData = MoveAbilityData{60.0f};
    npcData.actorData.motionData.gravityAbilityData = GravityAbilityData{};
    npcData.actorData.physicsBodyData.colliderSize = glm::vec2(8.0f, 13.0f);
    npcData.actorData.physicsBodyData.colliderOffset = glm::vec2(4.0f, 3.0f);
    BehaviorStateData patrolling;
    patrolling.name = "patrol";
    patrolling.patrolBehaviorData = PatrolBehaviorData();
    npcData.stateMachineBehaviorData = StateMachineBehaviorData{{patrolling}, {}};
    return npcData;
}

inline NpcSpawnData patrolling(
    std::string type,
    glm::ivec2 tilePosition,
    glm::ivec2 from,
    glm::ivec2 to)
{
    NpcSpawnData spawn = spawnAt(std::move(type), tilePosition);
    spawn.patrol = beatOf(from, to);
    return spawn;
}

inline constexpr int LedgeWidthTiles = 20;
inline constexpr int LedgeHeightTiles = 14;
inline constexpr int GroundRow = 12;
inline constexpr int LedgeRow = 6;
inline constexpr int LedgeLastTile = 6;
inline constexpr int StepRow = 8;
inline constexpr int StepFirstTile = 3;
inline constexpr int StepLastTile = 9;
inline constexpr int RiseRow = 10;
inline constexpr int RiseFirstTile = 12;
inline constexpr int RiseLastTile = 18;

inline constexpr int Solid = 1;
inline constexpr int Grippable = 2;
inline constexpr int WallTile = Grippable;
inline constexpr int CornerTile = Grippable;
inline constexpr int GroundTile = Solid;
inline constexpr int SurfaceTile = Solid;
inline constexpr int SurfaceStartTile = Solid;
inline constexpr int SurfaceEndTile = Solid;

inline constexpr glm::ivec2 LedgeLeftEnd{1, LedgeRow - 1};
inline constexpr glm::ivec2 LedgeRightEnd{LedgeLastTile, LedgeRow - 1};
inline constexpr glm::ivec2 OnTheGround{2, GroundRow - 1};
inline constexpr glm::ivec2 TopOfTheWall{1, 0};

inline TilePaletteData ledgePalette()
{
    TileData solid;
    solid.solid = true;
    TileData grippable;
    grippable.solid = grippable.grippable = true;
    return paletteOf({{0, TileData{}}, {Solid, solid}, {Grippable, grippable}});
}

inline Level levelWithALedgeAndAWall(const std::vector<NpcSpawnData> &npcs)
{
    TileMapData tileMapData;
    tileMapData.tilePalette = "default";
    tileMapData.indices =
        std::vector<std::vector<int>>(LedgeHeightTiles, std::vector<int>(LedgeWidthTiles, 0));
    std::vector<std::vector<int>> &indices = tileMapData.indices;

    for (int y = 0; y < GroundRow; ++y)
    {
        indices[y][0] = WallTile;
        indices[y][LedgeWidthTiles - 1] = WallTile;
    }

    for (int x = 0; x < LedgeWidthTiles; ++x)
    {
        indices[GroundRow][x] = GroundTile;
        indices[GroundRow + 1][x] = CornerTile;
    }
    indices[GroundRow][0] = CornerTile;
    indices[GroundRow][LedgeWidthTiles - 1] = CornerTile;

    indices[LedgeRow][0] = CornerTile;
    for (int x = 1; x < LedgeLastTile; ++x)
        indices[LedgeRow][x] = SurfaceTile;
    indices[LedgeRow][LedgeLastTile] = SurfaceEndTile;

    indices[StepRow][StepFirstTile] = SurfaceStartTile;
    for (int x = StepFirstTile + 1; x < StepLastTile; ++x)
        indices[StepRow][x] = SurfaceTile;
    indices[StepRow][StepLastTile] = SurfaceEndTile;

    indices[RiseRow][RiseFirstTile] = SurfaceStartTile;
    for (int x = RiseFirstTile + 1; x <= RiseLastTile; ++x)
        indices[RiseRow][x] = SurfaceTile;

    LevelData levelData;

    levelData.playerFeet = feetOf(glm::ivec2(0, 0));
    levelData.tileMapData = tileMapData;
    levelData.playerFeet = feetOf(OnTheGround);
    levelData.npcs = npcs;

    return Level(
        levelData,
        theOnlyPalette(ledgePalette()),
        loadGameData().playerData,
        shippedNpcData(),
        shippedPickupData());
}

inline float surfaceOf(int row)
{
    return static_cast<float>(row * 16);
}

inline void stepNpc(Npc &npc, const Level &level, int steps)
{
    for (int step = 0; step < steps; ++step)
    {
        npc.beginFrame();
        npc.fixedUpdate(0.01f, level);
    }
}

inline glm::vec2 footOf(const Npc &npc)
{
    return npc.body().aabb().bottomCenter();
}
