#include <catch2/catch_test_macros.hpp>
#include "animations/animation_manager.hpp"
#include "animations/sprite_animation.hpp"
#include "animations/sprite_animation_data.hpp"
#include "game/actor/actor_motion_state.hpp"

namespace
{
    SpriteAnimation animationOfFrame(int frame)
    {
        return SpriteAnimation(SpriteAnimationData(FrameAnimationData({frame}, 1.0f), 16, 16, 96));
    }

    ActorMotionState walkingOnGround()
    {
        ActorMotionState state;
        state.contacts.onGround = true;
        state.velocity = glm::vec2(50.0f, 0.0f);
        return state;
    }
} // namespace

TEST_CASE("Plays the animation for the state it is in", "[AnimationManager]")
{
    AnimationManager animationManager;
    animationManager.addAnimation(ActorAnimationState::Idle, animationOfFrame(1));
    animationManager.addAnimation(ActorAnimationState::Walk, animationOfFrame(2));

    animationManager.update(0.01f, walkingOnGround());

    REQUIRE(animationManager.getCurrentState() == ActorAnimationState::Walk);
}

TEST_CASE("Falls back to idle for a state it has no animation for", "[AnimationManager]")
{
    AnimationManager animationManager;
    animationManager.addAnimation(ActorAnimationState::Idle, animationOfFrame(1));

    REQUIRE_NOTHROW(animationManager.update(0.01f, walkingOnGround()));
    REQUIRE(animationManager.getCurrentState() == ActorAnimationState::Idle);
}

TEST_CASE("An actor without airborne animations survives being airborne", "[AnimationManager]")
{
    AnimationManager animationManager;
    animationManager.addAnimation(ActorAnimationState::Idle, animationOfFrame(1));

    ActorMotionState state;
    state.contacts.onGround = false;
    state.velocity = glm::vec2(0.0f, 40.0f);

    REQUIRE_NOTHROW(animationManager.update(0.01f, state));
    REQUIRE(animationManager.getCurrentState() == ActorAnimationState::Idle);

    state.dash.active = true;
    REQUIRE_NOTHROW(animationManager.update(0.01f, state));
    REQUIRE(animationManager.getCurrentState() == ActorAnimationState::Idle);
}
