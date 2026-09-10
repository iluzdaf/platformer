#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "animations/frame_animation_data.hpp"
#include "assets/sheet_data.hpp"
#include "pickups/pickup.hpp"
#include "pickups/pickup_data.hpp"
#include "pickups/pickup_spawn_data.hpp"

namespace
{
    constexpr int Cell = 16;

    PickupData spinning()
    {
        PickupData pickupData;
        pickupData.sheet = SheetData{"textures/somewhere.png", glm::ivec2(Cell)};
        pickupData.animationData = FrameAnimationData{{3, 8, 11}, 0.2f};
        return pickupData;
    }
}

TEST_CASE("A pickup said nothing about is drawn as big as its cell", "[Pickup]")
{
    PickupData wide = spinning();
    wide.sheet.cellSize = glm::ivec2(32, 24);

    Pickup pickup(PickupSpawnData{"coin", glm::vec2(0.0f)}, wide);

    REQUIRE(pickup.getSize() == glm::vec2(32.0f, 24.0f));
    REQUIRE(pickup.getAABB().size == glm::vec2(32.0f, 24.0f));
}

TEST_CASE("A pickup given a size is drawn at it, whatever its cell", "[Pickup]")
{
    PickupData shrunk = spinning();
    shrunk.sheet.cellSize = glm::ivec2(32);
    shrunk.size = glm::vec2(16.0f);

    Pickup pickup(PickupSpawnData{"coin", glm::vec2(0.0f)}, shrunk);

    REQUIRE(pickup.getSize() == glm::vec2(16.0f));
}

TEST_CASE("A pickup drawn as nothing is refused", "[Pickup]")
{
    PickupData nothing = spinning();
    nothing.sheet.cellSize = glm::ivec2(0);

    REQUIRE_THROWS_WITH(
        Pickup(PickupSpawnData{"coin", glm::vec2(0.0f)}, nothing),
        Catch::Matchers::ContainsSubstring("nobody can see"));
}

TEST_CASE("A pickup shows the frame its animation is on", "[Pickup]")
{
    Pickup pickup(PickupSpawnData{"coin", glm::vec2(0.0f)}, spinning());

    REQUIRE(pickup.frame() == 3);

    pickup.update(0.25f);

    REQUIRE(pickup.frame() == 8);
}

TEST_CASE("A pickup draws from the sheet its kind names", "[Pickup]")
{
    PickupData pickupData = spinning();
    pickupData.sheet = SheetData{"textures/somewhere.png", glm::ivec2(24, 32)};

    Pickup pickup(PickupSpawnData{"coin", glm::vec2(0.0f)}, pickupData);

    REQUIRE(pickup.getSheet() == pickupData.sheet);
}

TEST_CASE("A pickup stands where it was put and is as big as its kind", "[Pickup]")
{
    PickupData pickupData = spinning();
    pickupData.size = glm::vec2(8.0f, 12.0f);

    Pickup pickup(PickupSpawnData{"coin", glm::vec2(44.0f, 36.0f)}, pickupData);

    REQUIRE(pickup.getSpawn().feet == glm::vec2(44.0f, 36.0f));
    REQUIRE(pickup.getPosition() == glm::vec2(40.0f, 24.0f));
    REQUIRE(pickup.getSpawn().type == "coin");
    REQUIRE(pickup.getSize() == glm::vec2(8.0f, 12.0f));
}
