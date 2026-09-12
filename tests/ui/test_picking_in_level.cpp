#include <catch2/catch_test_macros.hpp>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "game/level.hpp"
#include "game/level_data.hpp"
#include "helpers/tile_positions.hpp"
#include "helpers/levels.hpp"
#include "helpers/floor_level.hpp"
#include "helpers/palettes.hpp"
#include "helpers/shipped.hpp"
#include "npc/npc.hpp"
#include "npc/npc_spawn_data.hpp"
#include "physics/aabb.hpp"
#include "physics/physics_body.hpp"
#include "pickups/pickup_spawn_data.hpp"
#include "player/player_data.hpp"
#include "ui/actors_in_level.hpp"
#include "ui/picking_in_level.hpp"

namespace
{
    Level levelHolding(
        const std::vector<NpcSpawnData> &npcs,
        const std::vector<PickupSpawnData> &pickups)
    {
        LevelData levelData = aFloorLevelPlacing(npcs);
        levelData.pickups = pickups;

        return Level(
            levelData,
            theOnlyPalette(aPaletteWithASolidTile()),
            PlayerData(),
            shippedNpcData(),
            shippedPickupData());
    }

    const AABB Nowhere{glm::vec2(-1000.0f), glm::vec2(1.0f)};
}

TEST_CASE("Nothing is at a place nothing stands", "[PickingInLevel]")
{
    Level level = levelHolding({}, {});

    REQUIRE(whatIsAt(level, Nowhere, glm::vec2(4.0f, 4.0f)) == ActorShown{});
}

TEST_CASE("A creature is picked out of the place it stands", "[PickingInLevel]")
{
    Level level = levelHolding({aRatAt(glm::ivec2(2, FloorLevelStanding))}, {});
    glm::vec2 middle = level.getNpcs().front()->body().aabb().center();

    REQUIRE(whatIsAt(level, Nowhere, middle) == ActorShown{ActorShown::What::Npc, 0});
}

TEST_CASE("A pickup is picked out of the place it waits", "[PickingInLevel]")
{
    Level level =
        levelHolding({}, {PickupSpawnData{"coin", feetOf(glm::ivec2(4, FloorLevelStanding))}});
    glm::vec2 middle = level.getPickups().front().getAABB().center();

    REQUIRE(whatIsAt(level, Nowhere, middle) == ActorShown{ActorShown::What::Pickup, 0});
}

TEST_CASE("A pickup already taken is not there to be picked", "[PickingInLevel]")
{
    Level level =
        levelHolding({}, {PickupSpawnData{"coin", feetOf(glm::ivec2(4, FloorLevelStanding))}});
    glm::vec2 middle = level.getPickups().front().getAABB().center();
    level.takePickupsTouching(level.getPickups().front().getAABB());

    REQUIRE(whatIsAt(level, Nowhere, middle) == ActorShown{});
}

TEST_CASE("The player is picked before whatever stands where they do", "[PickingInLevel]")
{
    Level level =
        levelHolding({}, {PickupSpawnData{"coin", feetOf(glm::ivec2(4, FloorLevelStanding))}});
    const AABB &coin = level.getPickups().front().getAABB();

    REQUIRE(whatIsAt(level, coin, coin.center()) == ActorShown{ActorShown::What::Player, 0});
}

TEST_CASE("The creature in front is the one picked", "[PickingInLevel]")
{
    glm::ivec2 sameTile(2, FloorLevelStanding);
    Level level = levelHolding({aRatAt(sameTile), aRatAt(sameTile)}, {});
    glm::vec2 middle = level.getNpcs().front()->body().aabb().center();

    REQUIRE(whatIsAt(level, Nowhere, middle) == ActorShown{ActorShown::What::Npc, 1});
}
