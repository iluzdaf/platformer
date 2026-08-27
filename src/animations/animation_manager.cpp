#include "animations/animation_manager.hpp"
#include "actor/actor_motion_state.hpp"

void AnimationManager::update(float deltaTime, const ActorMotionState &motionState)
{
    ActorAnimationState newState = currentState;

    if (motionState.dash.active)
        newState = ActorAnimationState::Dash;
    else if (!motionState.contacts.onGround)
    {
        if (motionState.wallSlide.active || motionState.climb.active)
            newState = ActorAnimationState::WallSlide;
        else if (motionState.velocity.y < 0.0f)
            newState = ActorAnimationState::Jump;
        else if (motionState.velocity.y > 0.0f)
            newState = ActorAnimationState::Fall;
    }
    else if (std::abs(motionState.velocity.x) > 0.1f)
        newState = ActorAnimationState::Walk;
    else
        newState = ActorAnimationState::Idle;

    if (!animations.contains(newState))
        newState = ActorAnimationState::Idle;

    if (newState != currentState)
    {
        currentState = newState;
        animations.at(currentState).reset();
    }

    animations.at(currentState).update(deltaTime);
}

const SpriteAnimation &AnimationManager::getCurrentAnimation()
{
    return animations.at(currentState);
}

ActorAnimationState AnimationManager::getCurrentState() const
{
    return currentState;
}

void AnimationManager::addAnimation(
    ActorAnimationState state,
    const SpriteAnimation &animation)
{
    animations.insert_or_assign(state, animation);
}