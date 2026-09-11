#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <algorithm>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "animations/frame_animation_data.hpp"
#include "assets/sheet_data.hpp"
#include "physics/aabb.hpp"
#include "pickups/collecting.hpp"
#include "pickups/pickup.hpp"
#include "pickups/pickup_data.hpp"
#include "pickups/pickup_spawn_data.hpp"

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
        return Pickup(PickupSpawnData{"coin", glm::vec2(x + 8.0f, 16.0f)}, worth(scoreDelta));
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
    REQUIRE(pickups.size() == 1);
    REQUIRE_FALSE(pickups[0].stillThere());
}

TEST_CASE("Several in one place are all taken at once", "[Collecting]")
{
    std::vector<Pickup> pickups{at(0.0f, 1), at(0.0f, 2), at(0.0f, 3)};

    std::vector<Pickup> taken = takeWhatTouches(pickups, reaching(0.0f));

    REQUIRE(taken.size() == 3);
    REQUIRE(pickups.size() == 3);
    REQUIRE(
        std::ranges::none_of(pickups, [](const Pickup &pickup) { return pickup.stillThere(); }));
}

TEST_CASE("Taking one leaves the others where they were", "[Collecting]")
{
    std::vector<Pickup> pickups{at(0.0f, 1), at(100.0f, 2), at(200.0f, 3)};

    std::vector<Pickup> taken = takeWhatTouches(pickups, reaching(0.0f));

    REQUIRE(taken.size() == 1);
    REQUIRE(taken[0].getScoreDelta() == 1);
    REQUIRE(pickups.size() == 3);
    REQUIRE_FALSE(pickups[0].stillThere());
    REQUIRE(pickups[1].stillThere());
    REQUIRE(pickups[2].stillThere());
}

TEST_CASE("What has been taken is not taken again", "[Collecting]")
{
    std::vector<Pickup> pickups{at(0.0f, 10)};

    REQUIRE(takeWhatTouches(pickups, reaching(0.0f)).size() == 1);
    REQUIRE(takeWhatTouches(pickups, reaching(0.0f)).empty());
}

TEST_CASE("A pickup is as big a box as its kind is", "[Collecting]")
{
    PickupData small = worth(1);
    small.size = glm::vec2(4.0f, 6.0f);

    Pickup pickup(PickupSpawnData{"coin", glm::vec2(22.0f, 36.0f)}, small);

    REQUIRE(pickup.getAABB().position == glm::vec2(20.0f, 30.0f));
    REQUIRE(pickup.getAABB().size == glm::vec2(4.0f, 6.0f));
}

TEST_CASE("A pickup with a collider is taken by that, not by what is drawn", "[Collecting]")
{
    PickupData glowing = worth(1);
    glowing.size = glm::vec2(32.0f);
    glowing.colliderSize = glm::vec2(8.0f);
    glowing.colliderOffset = glm::vec2(12.0f);

    Pickup pickup(PickupSpawnData{"heart", glm::vec2(16.0f, 32.0f)}, glowing);

    REQUIRE(pickup.getSize() == glm::vec2(32.0f));
    REQUIRE(pickup.getAABB().position == glm::vec2(12.0f));
    REQUIRE(pickup.getAABB().size == glm::vec2(8.0f));

    std::vector<Pickup> glow;
    glow.push_back(pickup);
    REQUIRE(takeWhatTouches(glow, AABB{glm::vec2(0.0f), glm::vec2(8.0f)}).empty());

    std::vector<Pickup> heart;
    heart.push_back(Pickup(PickupSpawnData{"heart", glm::vec2(16.0f, 32.0f)}, glowing));
    REQUIRE(takeWhatTouches(heart, AABB{glm::vec2(16.0f), glm::vec2(8.0f)}).size() == 1);
}

TEST_CASE("A pickup said nothing about is taken by the whole of what is drawn", "[Collecting]")
{
    PickupData plain = worth(1);
    plain.size = glm::vec2(24.0f, 10.0f);

    Pickup pickup(PickupSpawnData{"plain", glm::vec2(17.0f, 17.0f)}, plain);

    REQUIRE(pickup.getAABB().position == glm::vec2(5.0f, 7.0f));
    REQUIRE(pickup.getAABB().size == glm::vec2(24.0f, 10.0f));
}

TEST_CASE("A pickup nothing can reach is refused", "[Collecting]")
{
    PickupData unreachable = worth(1);
    unreachable.colliderSize = glm::vec2(0.0f, 8.0f);

    REQUIRE_THROWS_WITH(
        Pickup(PickupSpawnData{"coin", glm::vec2(0.0f)}, unreachable),
        Catch::Matchers::ContainsSubstring("nobody can take"));
}
