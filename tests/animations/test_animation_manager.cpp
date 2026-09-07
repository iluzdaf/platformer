#include <catch2/catch_test_macros.hpp>
#include "actor/actor_animation_state.hpp"
#include "animations/animation_manager.hpp"
#include "animations/frame_animation_data.hpp"
#include "animations/frame_animation.hpp"
#include "animations/frame_animation_data.hpp"
#include "actor/decided.hpp"
#include "actor/observed.hpp"

namespace
{
    FrameAnimation animationOfFrame(int frame)
    {
        return FrameAnimation(FrameAnimationData({frame}, 1.0f));
    }

    Observed walkingOnGround()
    {
        Observed observed;
        observed.contacts.onGround = true;
        observed.velocity = glm::vec2(50.0f, 0.0f);
        return observed;
    }
}

TEST_CASE("Plays the animation for the state it is in", "[AnimationManager]")
{
    AnimationManager animationManager;
    animationManager.addAnimation(ActorAnimationState::Idle, animationOfFrame(1));
    animationManager.addAnimation(ActorAnimationState::Walk, animationOfFrame(2));

    animationManager.update(0.01f, Decided{}, walkingOnGround());

    REQUIRE(animationManager.getCurrentState() == ActorAnimationState::Walk);
}

TEST_CASE("Falls back to idle for a state it has no animation for", "[AnimationManager]")
{
    AnimationManager animationManager;
    animationManager.addAnimation(ActorAnimationState::Idle, animationOfFrame(1));

    REQUIRE_NOTHROW(animationManager.update(0.01f, Decided{}, walkingOnGround()));
    REQUIRE(animationManager.getCurrentState() == ActorAnimationState::Idle);
}

TEST_CASE("An actor without airborne animations survives being airborne", "[AnimationManager]")
{
    AnimationManager animationManager;
    animationManager.addAnimation(ActorAnimationState::Idle, animationOfFrame(1));

    Decided state;

    Observed observed;
    observed.contacts.onGround = false;
    observed.velocity = glm::vec2(0.0f, 40.0f);

    REQUIRE_NOTHROW(animationManager.update(0.01f, state, observed));
    REQUIRE(animationManager.getCurrentState() == ActorAnimationState::Idle);

    state.dash.active = true;
    REQUIRE_NOTHROW(animationManager.update(0.01f, state, observed));
    REQUIRE(animationManager.getCurrentState() == ActorAnimationState::Idle);
}

TEST_CASE(
    "Off the ground, the observed velocity says whether it is a jump or a fall",
    "[AnimationManager]")
{
    AnimationManager animationManager;
    animationManager.addAnimation(ActorAnimationState::Idle, animationOfFrame(1));
    animationManager.addAnimation(ActorAnimationState::Jump, animationOfFrame(2));
    animationManager.addAnimation(ActorAnimationState::Fall, animationOfFrame(3));
    Observed airborne;
    airborne.contacts.onGround = false;

    airborne.velocity = glm::vec2(0.0f, -40.0f);
    animationManager.update(0.01f, Decided{}, airborne);
    REQUIRE(animationManager.getCurrentState() == ActorAnimationState::Jump);

    airborne.velocity = glm::vec2(0.0f, 40.0f);
    animationManager.update(0.01f, Decided{}, airborne);
    REQUIRE(animationManager.getCurrentState() == ActorAnimationState::Fall);
}
