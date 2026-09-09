#include <catch2/catch_test_macros.hpp>
#include "player/player_data.hpp"
#include "player/player.hpp"
#include "helpers/actors.hpp"
#include <stdexcept>
#include <set>
#include <string>
#include <string_view>
#include "actor/actor_animation_data.hpp"
#include "actor/actor_animation_state.hpp"
#include "actor/actor_animations.hpp"
#include "animations/frame_animation_data.hpp"

TEST_CASE("Every animation state has a slot that names it", "[ActorAnimations]")
{
    std::set<std::string_view> named;
    for (int value = 0; value < static_cast<int>(ActorAnimationState::Count); ++value)
    {
        const char *name = toString(static_cast<ActorAnimationState>(value));

        REQUIRE(std::string(name) != "Unknown");
        REQUIRE(named.insert(name).second);
    }
}

TEST_CASE("A state that is not one of them says so", "[ActorAnimations]")
{
    REQUIRE(std::string(toString(static_cast<ActorAnimationState>(99))) == "Unknown");
    REQUIRE(std::string(toString(ActorAnimationState::Count)) == "Unknown");
}

TEST_CASE("Idle is the one an actor always has, and the rest are said or not", "[ActorAnimations]")
{
    ActorAnimationData animations;
    animations.idle = FrameAnimationData({4}, 1.0f);
    animations.climb = FrameAnimationData({7}, 1.0f);

    for (const ActorAnimationSlot &slot : ActorAnimationSlots)
    {
        const FrameAnimationData *said = saidFor(animations, slot);
        if (slot.state == ActorAnimationState::Idle)
            REQUIRE(said == &animations.idle);
        else if (slot.state == ActorAnimationState::Climb)
            REQUIRE(said == &animations.climb.value());
        else
            REQUIRE(said == nullptr);
    }
}

TEST_CASE("A slot reaches the field its name spells", "[ActorAnimations]")
{
    ActorAnimationData animations;
    animations.knockback = FrameAnimationData({2}, 1.0f);
    animations.wallSlide = FrameAnimationData({3}, 1.0f);

    for (const ActorAnimationSlot &slot : ActorAnimationSlots)
    {
        if (std::string_view(slot.name) == "knockback")
            REQUIRE(saidFor(animations, slot) == &animations.knockback.value());

        if (std::string_view(slot.name) == "wallSlide")
            REQUIRE(saidFor(animations, slot) == &animations.wallSlide.value());
    }
}

TEST_CASE("An actor with pictures to choose from and no ladder is refused", "[ActorAnimations]")
{
    PlayerData playerData;
    playerData.actorData.animationData.idle = FrameAnimationData({0}, 1.0f);
    playerData.actorData.animationData.walk = FrameAnimationData({1}, 1.0f);

    REQUIRE_THROWS_AS(Player(playerData, noIntentions()), std::runtime_error);
}

TEST_CASE("An actor with only an idle picture needs no ladder", "[ActorAnimations]")
{
    PlayerData playerData;
    playerData.actorData.animationData.idle = FrameAnimationData({0}, 1.0f);

    REQUIRE_NOTHROW(Player(playerData, noIntentions()));
}
