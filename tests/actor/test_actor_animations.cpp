#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "animations/animator_data.hpp"
#include "animations/animator_data.hpp"
#include "conditions/asked.hpp"
#include "animations/frame_animation_data.hpp"
#include "helpers/actors.hpp"
#include "player/player.hpp"
#include "player/player_data.hpp"

TEST_CASE("A clip is found by its name, and a name nobody drew is nobody's", "[ActorAnimations]")
{
    AnimatorData animations;
    animations.clips["idle"] = FrameAnimationData({4}, 1.0f);
    animations.clips["climb"] = FrameAnimationData({7}, 1.0f);

    REQUIRE(clipNamed(animations, "climb") == &animations.clips.at("climb"));
    REQUIRE(clipNamed(animations, "idle") == &animations.clips.at("idle"));
    REQUIRE(clipNamed(animations, "somersault") == nullptr);
}

TEST_CASE("A clip it neither starts in nor has a rule to show is refused", "[ActorAnimations]")
{
    PlayerData playerData;
    AnimatorData &animations = playerData.actorData.animationData.emplace();
    animations.startClip = "idle";
    animations.clips["idle"] = FrameAnimationData({0}, 1.0f);
    animations.clips["walk"] = FrameAnimationData({1}, 1.0f);

    REQUIRE_THROWS_WITH(
        Player(playerData, noIntentions()),
        Catch::Matchers::ContainsSubstring("never shows \"walk\""));
}

TEST_CASE("An actor with only the clip it starts in needs no rules", "[ActorAnimations]")
{
    PlayerData playerData;
    AnimatorData &animations = playerData.actorData.animationData.emplace();
    animations.startClip = "idle";
    animations.clips["idle"] = FrameAnimationData({0}, 1.0f);

    REQUIRE_NOTHROW(Player(playerData, noIntentions()));
}

TEST_CASE("A rule showing a clip the actor does not have is refused", "[ActorAnimations]")
{
    PlayerData playerData;
    AnimatorData &animations = playerData.actorData.animationData.emplace();
    animations.startClip = "idle";
    animations.clips["idle"] = FrameAnimationData({0}, 1.0f);
    animations.clips["walk"] = FrameAnimationData({1}, 1.0f);
    AnimationWhenData moving;
    moving["moving"] = true;
    animations.rules = {{"walk", moving}, {"somersault", moving}};

    REQUIRE_THROWS_WITH(
        Player(playerData, noIntentions()),
        Catch::Matchers::ContainsSubstring(
            "a rule showing \"somersault\", and there is no such clip"));
}

TEST_CASE("A creature named with a clip nobody else has can show it", "[ActorAnimations]")
{
    PlayerData playerData;
    AnimatorData &animations = playerData.actorData.animationData.emplace();
    animations.startClip = "idle";
    animations.clips["idle"] = FrameAnimationData({0}, 1.0f);
    animations.clips["somersault"] = FrameAnimationData({1}, 1.0f);
    AnimationWhenData airborne;
    airborne["onGround"] = false;
    animations.rules = {{"somersault", airborne}};

    REQUIRE_NOTHROW(Player(playerData, noIntentions()));
}

TEST_CASE("A rule asking about a fact nobody publishes is refused", "[ActorAnimations]")
{
    PlayerData playerData;
    AnimatorData &animations = playerData.actorData.animationData.emplace();
    animations.startClip = "idle";
    animations.clips["idle"] = FrameAnimationData({0}, 1.0f);
    animations.clips["walk"] = FrameAnimationData({1}, 1.0f);
    AnimationWhenData snowing;
    snowing["snowing"] = true;
    animations.rules = {{"walk", snowing}};

    REQUIRE_THROWS_WITH(
        Player(playerData, noIntentions()),
        Catch::Matchers::ContainsSubstring("asks about \"snowing\", and there is no such fact"));
}
