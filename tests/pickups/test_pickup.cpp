#include <catch2/catch_test_macros.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "animations/frame_animation_data.hpp"
#include "assets/sheet.hpp"
#include "pickups/pickup.hpp"
#include "pickups/pickup_data.hpp"

namespace
{
    constexpr int Cell = 16;

    PickupData spinning()
    {
        PickupData pickupData;
        pickupData.sheet = Sheet{"textures/tile_set.png", glm::ivec2(Cell)};
        pickupData.animationData = FrameAnimationData{{3, 8, 11}, 0.2f};
        return pickupData;
    }
}

TEST_CASE("A pickup shows the frame its animation is on", "[Pickup]")
{
    Pickup pickup(spinning(), glm::vec2(0.0f));

    REQUIRE(pickup.getCurrentFrame() == 3);

    pickup.update(0.25f);

    REQUIRE(pickup.getCurrentFrame() == 8);
}

TEST_CASE("A pickup draws from the sheet its kind names", "[Pickup]")
{
    PickupData pickupData = spinning();
    pickupData.sheet = Sheet{"textures/somewhere.png", glm::ivec2(24, 32)};

    Pickup pickup(pickupData, glm::vec2(0.0f));

    REQUIRE(pickup.getSheet() == pickupData.sheet);
}

TEST_CASE("A pickup stands where it was put and is as big as its kind", "[Pickup]")
{
    PickupData pickupData = spinning();
    pickupData.size = glm::vec2(8.0f, 12.0f);

    Pickup pickup(pickupData, glm::vec2(40.0f, 24.0f));

    REQUIRE(pickup.getPosition() == glm::vec2(40.0f, 24.0f));
    REQUIRE(pickup.getSize() == glm::vec2(8.0f, 12.0f));
}
