#include <catch2/catch_test_macros.hpp>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "animations/frame_animation_data.hpp"
#include "assets/sheet_data.hpp"
#include "physics/aabb.hpp"
#include "pickups/collecting.hpp"
#include "pickups/pickup.hpp"
#include "pickups/pickup_data.hpp"

namespace
{
    PickupData worth(int scoreDelta)
    {
        PickupData pickupData;
        pickupData.sheet = SheetData{"textures/somewhere.png", glm::ivec2(16)};
        pickupData.animationData = FrameAnimationData{{0}, 1.0f};
        pickupData.size = glm::vec2(16.0f);
        pickupData.scoreDelta = scoreDelta;
        return pickupData;
    }

    Pickup at(float x, int scoreDelta = 1)
    {
        return Pickup(worth(scoreDelta), glm::vec2(x, 0.0f));
    }

    AABB reaching(float x)
    {
        return AABB{glm::vec2(x, 0.0f), glm::vec2(16.0f)};
    }
}

TEST_CASE("Nothing is taken from where nothing is", "[Collecting]")
{
    std::vector<Pickup> pickups;

    REQUIRE(takeWhatTouches(pickups, reaching(0.0f)).empty());
}

TEST_CASE("What is out of reach is left where it is", "[Collecting]")
{
    std::vector<Pickup> pickups{at(100.0f)};

    REQUIRE(takeWhatTouches(pickups, reaching(0.0f)).empty());
    REQUIRE(pickups.size() == 1);
}

TEST_CASE("What is touched is handed over and taken away", "[Collecting]")
{
    std::vector<Pickup> pickups{at(0.0f, 10)};

    std::vector<Pickup> taken = takeWhatTouches(pickups, reaching(8.0f));

    REQUIRE(taken.size() == 1);
    REQUIRE(taken[0].getScoreDelta() == 10);
    REQUIRE(pickups.empty());
}

TEST_CASE("Several in one place are all taken at once", "[Collecting]")
{
    std::vector<Pickup> pickups{at(0.0f, 1), at(0.0f, 2), at(0.0f, 3)};

    std::vector<Pickup> taken = takeWhatTouches(pickups, reaching(0.0f));

    REQUIRE(taken.size() == 3);
    REQUIRE(pickups.empty());
}

TEST_CASE("Taking one leaves the others where they were", "[Collecting]")
{
    std::vector<Pickup> pickups{at(0.0f, 1), at(100.0f, 2), at(200.0f, 3)};

    std::vector<Pickup> taken = takeWhatTouches(pickups, reaching(0.0f));

    REQUIRE(taken.size() == 1);
    REQUIRE(taken[0].getScoreDelta() == 1);
    REQUIRE(pickups.size() == 2);
    REQUIRE(pickups[0].getScoreDelta() == 2);
    REQUIRE(pickups[1].getScoreDelta() == 3);
}

TEST_CASE("A pickup is as big a box as its kind is", "[Collecting]")
{
    PickupData small = worth(1);
    small.size = glm::vec2(4.0f, 6.0f);

    Pickup pickup(small, glm::vec2(20.0f, 30.0f));

    REQUIRE(pickup.getAABB().position == glm::vec2(20.0f, 30.0f));
    REQUIRE(pickup.getAABB().size == glm::vec2(4.0f, 6.0f));
}
