#include <algorithm>
#include <cstddef>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "game/game_data.hpp"
#include "game/level.hpp"
#include "game/level_data.hpp"
#include "helpers/actors.hpp"
#include "helpers/palettes.hpp"
#include "helpers/player_fixtures.hpp"
#include "helpers/tile_positions.hpp"
#include "input/input_intentions.hpp"
#include "player/player.hpp"
#include "tile_map/tile_collider_data.hpp"
#include "tile_map/tile_data.hpp"
#include "tile_map/tile_map.hpp"
#include "tile_map/tile_map_data.hpp"
#include "tile_map/tile_palette_data.hpp"
#include <tuple>
#include "timing/fixed_time_step.hpp"

namespace
{
    constexpr int PitMapWidth = 30;
    constexpr int PitMapHeight = 12;
    constexpr int PitFloorRow = 8;
    constexpr int PitHazardRow = 7;
    constexpr int PitStart = 12;

    enum class Pit
    {
        Spikes,
        Hole,
        StepUp
    };

    constexpr int PitSolid = 1;
    constexpr int PitSpike = 2;

    TilePalettes pitPalette()
    {
        TileData solid;
        solid.solid = true;
        TileData spike;
        spike.deadly = true;
        spike.collider = TileColliderData{glm::vec2(0.0f, 12.0f), glm::vec2(16.0f, 4.0f)};
        return theOnlyPalette(paletteOf({{0, TileData{}}, {PitSolid, solid}, {PitSpike, spike}}));
    }

    TileMapData pitOf(int tiles, Pit kind)
    {
        int solidTile = PitSolid;
        int spikeTile = PitSpike;

        std::vector<std::vector<int>> rows(PitMapHeight, std::vector<int>(PitMapWidth, 0));
        for (int x = 0; x < PitMapWidth; ++x)
            rows[PitFloorRow][x] = solidTile;

        if (kind == Pit::StepUp)
            for (int x = PitStart; x < PitMapWidth; ++x)
                for (int y = std::max(0, PitFloorRow - tiles); y < PitMapHeight; ++y)
                    rows[y][x] = solidTile;
        else
            for (int x = PitStart; x < PitStart + tiles; ++x)
            {
                if (kind == Pit::Spikes)
                    rows[PitHazardRow][x] = spikeTile;
                else
                    rows[PitFloorRow][x] = 0;
            }

        TileMapData tileMapData;
        tileMapData.indices = rows;
        tileMapData.tilePalette = "default";
        return tileMapData;
    }

    bool getsAcross(const GameData &gameData, int tiles, Pit kind, bool jump, bool dash)
    {
        LevelData levelData;
        levelData.playerFeet = feetOf(glm::ivec2(0, 0));
        levelData.tileMapData = pitOf(tiles, kind);
        Level level(
            levelData, pitPalette(), gameData.playerData, gameData.npcData, gameData.pickupData);

        float pitLeft = static_cast<float>(PitStart) * 16.0f;
        float pitRight = static_cast<float>(PitStart + tiles) * 16.0f;
        float floorY = static_cast<float>(PitFloorRow) * 16.0f;

        for (float triggerAt = pitLeft - 80.0f; triggerAt <= pitLeft; triggerAt += 1.0f)
            for (float dashAfter = 0.0f; dashAfter <= (jump && dash ? 0.45f : 0.0f);
                 dashAfter += 0.025f)
            {
                ScriptedIntentions input;
                Player player(gameData.playerData, input);
                player.standAt(glm::vec2(4.0f * 16.0f + 8.0f, floorY));

                FixedTimeStep timestepper;
                float triggered = -1.0f;

                for (int frame = 0; frame < 240; ++frame)
                {
                    float now = frame / 60.0f;
                    InputIntentions intentions;
                    intentions.direction.x = 1.0f;

                    if (triggered < 0.0f && player.body().position().x + 8.0f >= triggerAt)
                    {
                        triggered = now;
                        intentions.jumpRequested = jump;
                        intentions.dashRequested = dash && dashAfter <= 0.0f;
                    }
                    else if (triggered >= 0.0f)
                    {
                        intentions.jumpHeld = jump && now - triggered < 0.25f;
                        intentions.dashRequested = dash && dashAfter > 0.0f &&
                                                   now - triggered >= dashAfter &&
                                                   now - triggered < dashAfter + 1.0f / 60.0f;
                    }
                    input.set(intentions);

                    runFor(player, level, 1.0f / 60.0f, timestepper);

                    glm::vec2 position = player.body().position();
                    if (kind == Pit::StepUp)
                    {
                        float ledgeY = static_cast<float>(PitFloorRow - tiles) * 16.0f;
                        if (player.observed().contacts.onGround &&
                            position.y + 16.0f <= ledgeY + 0.5f)
                            return true;
                        if (position.y > floorY)
                            break;
                        continue;
                    }

                    bool onSpikes = kind == Pit::Spikes && position.x + 12.0f > pitLeft &&
                                    position.x + 4.0f < pitRight &&
                                    position.y + 16.0f > static_cast<float>(PitHazardRow) * 16.0f &&
                                    position.y + 3.0f < floorY;
                    if (onSpikes || position.y > floorY)
                        break;

                    if (player.observed().contacts.onGround && position.x + 4.0f > pitRight)
                        return true;
                }
            }

        return false;
    }
}

TEST_CASE("The shipped player's jump is worth three tiles", "[Player][Tuning]")
{
    GameData gameData = loadGameData();

    REQUIRE(getsAcross(gameData, 3, Pit::StepUp, true, false));
    REQUIRE_FALSE(getsAcross(gameData, 4, Pit::StepUp, true, false));

    REQUIRE(getsAcross(gameData, 3, Pit::Hole, true, false));
    REQUIRE_FALSE(getsAcross(gameData, 4, Pit::Hole, true, false));

    REQUIRE(getsAcross(gameData, 2, Pit::Spikes, true, false));
    REQUIRE_FALSE(getsAcross(gameData, 3, Pit::Spikes, true, false));
}

TEST_CASE("The shipped player's dash is worth four tiles, and no spikes", "[Player][Tuning]")
{
    GameData gameData = loadGameData();

    REQUIRE(getsAcross(gameData, 4, Pit::Hole, false, true));
    REQUIRE_FALSE(getsAcross(gameData, 5, Pit::Hole, false, true));

    REQUIRE_FALSE(getsAcross(gameData, 1, Pit::Spikes, false, true));
}

TEST_CASE("The shipped player's jump and dash together are worth five tiles", "[Player][Tuning]")
{
    GameData gameData = loadGameData();

    REQUIRE(getsAcross(gameData, 5, Pit::Hole, true, true));
    REQUIRE_FALSE(getsAcross(gameData, 6, Pit::Hole, true, true));

    REQUIRE(getsAcross(gameData, 4, Pit::Spikes, true, true));
    REQUIRE_FALSE(getsAcross(gameData, 5, Pit::Spikes, true, true));

    REQUIRE(gameData.playerData.actorData.motionData.dashAbilityData->airborneFraction < 1.0f);
}

namespace
{
    constexpr int StairsSolid = 1;
    constexpr int StairsWall = 2;

    TilePalettes stairsPalette()
    {
        TileData solid;
        solid.solid = true;
        TileData wall;
        wall.solid = wall.grippable = true;
        return theOnlyPalette(
            paletteOf({{0, TileData{}}, {StairsSolid, solid}, {StairsWall, wall}}));
    }

    TileMapData threeStairs()
    {
        TileMapData tileMapData;
        tileMapData.tilePalette = "default";
        tileMapData.indices = std::vector<std::vector<int>>(14, std::vector<int>(20, 0));
        std::vector<std::vector<int>> &rows = tileMapData.indices;

        for (int y = 0; y < 12; ++y)
            rows[y][0] = rows[y][19] = StairsWall;
        for (int x = 0; x < 20; ++x)
            rows[12][x] = rows[13][x] = StairsSolid;

        for (int x = 12; x <= 18; ++x)
            rows[10][x] = StairsSolid;
        for (int x = 3; x <= 9; ++x)
            rows[8][x] = StairsSolid;
        for (int x = 1; x <= 6; ++x)
            rows[6][x] = StairsSolid;

        return tileMapData;
    }
}

TEST_CASE("The shipped player can climb three stepped platforms", "[Player][Tuning]")
{
    GameData gameData = loadGameData();
    LevelData levelData;
    levelData.playerFeet = feetOf(glm::ivec2(1, 11));
    levelData.tileMapData = threeStairs();
    Level level(levelData, stairsPalette(), gameData.playerData, {}, {});

    struct Step
    {
        const char *what;
        float edgeX, standOn, towards, intoPlatform;
        int landOnRow;
    };

    const TileMap &tileMap = level.getTileMap();
    for (Step step :
         {Step{"floor to the lowest platform", 192.0f, 192.0f, 1.0f, -1.0f, 10},
          Step{"lowest to the middle platform", 192.0f, 160.0f, -1.0f, 1.0f, 8},
          Step{"middle to the highest platform", 160.0f, 128.0f, -1.0f, -1.0f, 6}})
    {
        int takeOffPointsThatWork = 0;
        for (float back = 0.0f; back <= 44.0f; back += 2.0f)
        {
            ScriptedIntentions input;
            Player player(gameData.playerData, input);
            player.standAt(glm::vec2(step.edgeX + step.intoPlatform * back, step.standOn));

            FixedTimeStep timestepper;
            for (int frame = 0; frame < 150; ++frame)
            {
                InputIntentions intentions;
                intentions.jumpRequested = frame == 0;
                intentions.jumpHeld = frame < 20;
                intentions.direction.x = step.towards;
                input.set(intentions);

                runFor(player, level, 1.0f / 60.0f, timestepper);

                glm::vec2 feet = player.body().aabb().bottomCenter();
                if (player.observed().contacts.onGround &&
                    tileMap.tileStoodOnAt(feet).y == step.landOnRow - 1)
                {
                    ++takeOffPointsThatWork;
                    break;
                }
            }
        }

        INFO(step.what << " worked from " << takeOffPointsThatWork << " take off points");
        REQUIRE(takeOffPointsThatWork >= 5);
    }
}

namespace
{
    constexpr int DropMapWidth = 6;

    bool shakenAfterFallingTiles(const GameData &gameData, int tiles)
    {
        int rows = tiles + 3;
        std::vector<std::vector<int>> indices(
            static_cast<std::size_t>(rows), std::vector<int>(DropMapWidth, 0));
        for (int x = 0; x < DropMapWidth; ++x)
            indices[static_cast<std::size_t>(rows) - 1][x] = PitSolid;

        LevelData levelData;
        levelData.tileMapData.indices = indices;
        levelData.tileMapData.tilePalette = "default";
        levelData.playerFeet = feetOf(glm::ivec2(2, rows - 2));
        Level level(
            levelData, pitPalette(), gameData.playerData, gameData.npcData, gameData.pickupData);

        ScriptedIntentions input;
        Player player(gameData.playerData, input);
        bool shaken = false;
        std::ignore = player.onFallFromHeight.connect([&shaken] { shaken = true; });
        player.standAt(feetOf(glm::ivec2(2, rows - 2 - tiles)));

        FixedTimeStep timestepper;
        for (int frame = 0; frame < 240; ++frame)
            runFor(player, level, 1.0f / 60.0f, timestepper);

        REQUIRE(player.feet().y == feetOf(glm::ivec2(2, rows - 2)).y);

        return shaken;
    }
}

TEST_CASE("The shipped player's fall is worth twelve tiles before it shakes", "[Player][Tuning]")
{
    GameData gameData = loadGameData();

    REQUIRE(shakenAfterFallingTiles(gameData, 12));
    REQUIRE_FALSE(shakenAfterFallingTiles(gameData, 11));
}
