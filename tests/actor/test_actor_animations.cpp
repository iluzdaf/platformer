#include <catch2/catch_test_macros.hpp>
#include <stdexcept>
#include "actor/actor_animation_data.hpp"
#include "animations/animator_data.hpp"
#include "conditions/asked.hpp"
#include "animations/frame_animation_data.hpp"
#include "helpers/actors.hpp"
#include "player/player.hpp"
#include "player/player_data.hpp"

TEST_CASE("A clip is found by its name, and a name nobody drew is nobody's", "[ActorAnimations]")
{
    ActorAnimationData animations;
    animations.clips["idle"] = FrameAnimationData({4}, 1.0f);
    animations.clips["climb"] = FrameAnimationData({7}, 1.0f);

    REQUIRE(clipNamed(animations, "climb") == &animations.clips.at("climb"));
    REQUIRE(clipNamed(animations, IdleClip) == &animations.clips.at("idle"));
    REQUIRE(clipNamed(animations, "somersault") == nullptr);
}

TEST_CASE("An actor with pictures to choose from and no ladder is refused", "[ActorAnimations]")
{
    PlayerData playerData;
    playerData.actorData.animationData.clips["idle"] = FrameAnimationData({0}, 1.0f);
    playerData.actorData.animationData.clips["walk"] = FrameAnimationData({1}, 1.0f);

    REQUIRE_THROWS_AS(Player(playerData, noIntentions()), std::runtime_error);
}

TEST_CASE("An actor with only an idle picture needs no ladder", "[ActorAnimations]")
{
    PlayerData playerData;
    playerData.actorData.animationData.clips["idle"] = FrameAnimationData({0}, 1.0f);

    REQUIRE_NOTHROW(Player(playerData, noIntentions()));
}

TEST_CASE("A ladder naming a clip the actor does not have is refused", "[ActorAnimations]")
{
    PlayerData playerData;
    playerData.actorData.animationData.clips["idle"] = FrameAnimationData({0}, 1.0f);
    playerData.actorData.animationData.clips["walk"] = FrameAnimationData({1}, 1.0f);
    AnimationWhenData moving;
    moving["moving"] = true;
    playerData.actorData.animationData.ladder = AnimatorData{{{"", "somersault", moving}}};

    REQUIRE_THROWS_AS(Player(playerData, noIntentions()), std::runtime_error);

    playerData.actorData.animationData.ladder = AnimatorData{{{"somersault", "walk", moving}}};

    REQUIRE_THROWS_AS(Player(playerData, noIntentions()), std::runtime_error);
}

TEST_CASE("A creature named with a clip nobody else has can show it", "[ActorAnimations]")
{
    PlayerData playerData;
    playerData.actorData.animationData.clips["idle"] = FrameAnimationData({0}, 1.0f);
    playerData.actorData.animationData.clips["somersault"] = FrameAnimationData({1}, 1.0f);
    AnimationWhenData airborne;
    airborne["onGround"] = false;
    playerData.actorData.animationData.ladder = AnimatorData{{{"", "somersault", airborne}}};

    REQUIRE_NOTHROW(Player(playerData, noIntentions()));
}

TEST_CASE("A rung asking about a fact nobody publishes is refused", "[ActorAnimations]")
{
    PlayerData playerData;
    playerData.actorData.animationData.clips["idle"] = FrameAnimationData({0}, 1.0f);
    playerData.actorData.animationData.clips["walk"] = FrameAnimationData({1}, 1.0f);
    AnimationWhenData snowing;
    snowing["snowing"] = true;
    playerData.actorData.animationData.ladder = AnimatorData{{{"", "walk", snowing}}};

    REQUIRE_THROWS_AS(Player(playerData, noIntentions()), std::runtime_error);
}
