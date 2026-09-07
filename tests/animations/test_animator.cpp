#include <catch2/catch_test_macros.hpp>
#include "actor/actor_animation_state.hpp"
#include "animations/animator.hpp"
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

TEST_CASE("Plays the animation for the state it is in", "[Animator]")
{
    Animator animator;
    animator.add(ActorAnimationState::Idle, animationOfFrame(1));
    animator.add(ActorAnimationState::Walk, animationOfFrame(2));

    animator.animate(0.01f, Decided{}, walkingOnGround());

    REQUIRE(animator.state() == ActorAnimationState::Walk);
}

TEST_CASE("Falls back to idle for a state it has no animation for", "[Animator]")
{
    Animator animator;
    animator.add(ActorAnimationState::Idle, animationOfFrame(1));

    REQUIRE_NOTHROW(animator.animate(0.01f, Decided{}, walkingOnGround()));
    REQUIRE(animator.state() == ActorAnimationState::Idle);
}

TEST_CASE("An actor without airborne animations survives being airborne", "[Animator]")
{
    Animator animator;
    animator.add(ActorAnimationState::Idle, animationOfFrame(1));

    Decided decided;

    Observed observed;
    observed.contacts.onGround = false;
    observed.velocity = glm::vec2(0.0f, 40.0f);

    REQUIRE_NOTHROW(animator.animate(0.01f, decided, observed));
    REQUIRE(animator.state() == ActorAnimationState::Idle);

    decided.dash.active = true;
    REQUIRE_NOTHROW(animator.animate(0.01f, decided, observed));
    REQUIRE(animator.state() == ActorAnimationState::Idle);
}

TEST_CASE("Off the ground, the observed velocity says whether it is a jump or a fall", "[Animator]")
{
    Animator animator;
    animator.add(ActorAnimationState::Idle, animationOfFrame(1));
    animator.add(ActorAnimationState::Jump, animationOfFrame(2));
    animator.add(ActorAnimationState::Fall, animationOfFrame(3));
    Observed airborne;
    airborne.contacts.onGround = false;

    airborne.velocity = glm::vec2(0.0f, -40.0f);
    animator.animate(0.01f, Decided{}, airborne);
    REQUIRE(animator.state() == ActorAnimationState::Jump);

    airborne.velocity = glm::vec2(0.0f, 40.0f);
    animator.animate(0.01f, Decided{}, airborne);
    REQUIRE(animator.state() == ActorAnimationState::Fall);
}
