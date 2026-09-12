#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "navigation/navigation_profile.hpp"
#include "navigation/navigation_profile_builder.hpp"
#include "actor/actor_data.hpp"
#include "actor/abilities/wall_climb_ability_data.hpp"
#include "actor/abilities/wall_hang_ability_data.hpp"

TEST_CASE("Climbing takes both holding on and moving", "[NavigationProfile]")
{
    ActorData actorData;

    SECTION("Neither is not climbing")
    {
        REQUIRE_FALSE(buildNavigationProfile(actorData).climbs());
    }

    SECTION("Holding on without moving is not climbing")
    {
        actorData.motionData.wallHangAbilityData = WallHangAbilityData();

        REQUIRE_FALSE(buildNavigationProfile(actorData).climbs());
    }

    SECTION("Moving without holding on is not climbing, it is falling off")
    {
        actorData.motionData.wallClimbAbilityData = WallClimbAbilityData();

        REQUIRE_FALSE(buildNavigationProfile(actorData).climbs());
    }

    SECTION("Both is climbing")
    {
        actorData.motionData.wallHangAbilityData = WallHangAbilityData();
        actorData.motionData.wallClimbAbilityData = WallClimbAbilityData();

        REQUIRE(buildNavigationProfile(actorData).climbs());
    }
}

TEST_CASE("A profile of a body with no size is refused, not walked with", "[NavigationProfile]")
{
    ActorData actorData;
    actorData.physicsBodyData.colliderSize = glm::vec2(0.0f);

    REQUIRE_THROWS_WITH(
        buildNavigationProfile(actorData),
        Catch::Matchers::ContainsSubstring("collider of no size"));
}
