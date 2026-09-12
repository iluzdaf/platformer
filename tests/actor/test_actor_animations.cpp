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

TEST_CASE("An actor with pictures to choose from and no ladder is refused", "[ActorAnimations]")
{
    PlayerData playerData;
    AnimatorData &animations = playerData.actorData.animationData.emplace();
    animations.startClip = "idle";
    animations.clips["idle"] = FrameAnimationData({0}, 1.0f);
    animations.clips["walk"] = FrameAnimationData({1}, 1.0f);

    REQUIRE_THROWS_WITH(
        Player(playerData, noIntentions()),
        Catch::Matchers::ContainsSubstring(
            "\"walk\" is neither where it starts nor on the ladder"));
}

TEST_CASE("An actor with only an idle picture needs no ladder", "[ActorAnimations]")
{
    PlayerData playerData;
    AnimatorData &animations = playerData.actorData.animationData.emplace();
    animations.startClip = "idle";
    animations.clips["idle"] = FrameAnimationData({0}, 1.0f);

    REQUIRE_NOTHROW(Player(playerData, noIntentions()));
}

TEST_CASE("A ladder naming a clip the actor does not have is refused", "[ActorAnimations]")
{
    PlayerData playerData;
    AnimatorData &animations = playerData.actorData.animationData.emplace();
    animations.startClip = "idle";
    animations.clips["idle"] = FrameAnimationData({0}, 1.0f);
    animations.clips["walk"] = FrameAnimationData({1}, 1.0f);
    AnimationWhenData moving;
    moving["moving"] = true;
    animations.ladder = AnimationLadderData{{{"", "walk", moving}, {"", "somersault", moving}}};

    REQUIRE_THROWS_WITH(
        Player(playerData, noIntentions()),
        Catch::Matchers::ContainsSubstring("goes to \"somersault\", and there is no such clip"));

    animations.ladder = AnimationLadderData{{{"somersault", "walk", moving}}};

    REQUIRE_THROWS_WITH(
        Player(playerData, noIntentions()),
        Catch::Matchers::ContainsSubstring(
            "leaves from \"somersault\", and there is no such clip"));
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
    animations.ladder = AnimationLadderData{{{"", "somersault", airborne}}};

    REQUIRE_NOTHROW(Player(playerData, noIntentions()));
}

TEST_CASE("A rung asking about a fact nobody publishes is refused", "[ActorAnimations]")
{
    PlayerData playerData;
    AnimatorData &animations = playerData.actorData.animationData.emplace();
    animations.startClip = "idle";
    animations.clips["idle"] = FrameAnimationData({0}, 1.0f);
    animations.clips["walk"] = FrameAnimationData({1}, 1.0f);
    AnimationWhenData snowing;
    snowing["snowing"] = true;
    animations.ladder = AnimationLadderData{{{"", "walk", snowing}}};

    REQUIRE_THROWS_WITH(
        Player(playerData, noIntentions()),
        Catch::Matchers::ContainsSubstring("asks about \"snowing\", and there is no such fact"));
}
