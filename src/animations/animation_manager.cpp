#include "animations/animation_manager.hpp"
#include "game/player/player_state.hpp"

void AnimationManager::update(float deltaTime, const PlayerState &playerState)
{
    PlayerAnimationState newState = currentState;

    if (playerState.dashing)
    {
        newState = PlayerAnimationState::Dash;
    }
    else if (!playerState.onGround)
    {
        if (playerState.velocity.y < 0.0f)
        {
            newState = PlayerAnimationState::Jump;
        }
        else if (playerState.wallSliding)
        {
            newState = PlayerAnimationState::WallSlide;
        }
        else if (playerState.velocity.y > 0.0f)
        {
            newState = PlayerAnimationState::Fall;
        }
    }
    else if (std::abs(playerState.velocity.x) > 0.1f)
    {
        newState = PlayerAnimationState::Walk;
    }
    else
    {
        newState = PlayerAnimationState::Idle;
    }

    if (newState != currentState)
    {
        currentState = newState;
        getCurrentAnimation().reset();
    }

    getCurrentAnimation().update(deltaTime);
}

SpriteAnimation &AnimationManager::getCurrentAnimation()
{
    return animations.at(currentState);
}

PlayerAnimationState AnimationManager::getCurrentState() const
{
    return currentState;
}

void AnimationManager::addAnimation(
    PlayerAnimationState state,
    const SpriteAnimation &animation)
{
    animations.insert_or_assign(state, animation);
}

void AnimationManager::reset()
{
    currentState = PlayerAnimationState::Idle;

    for (auto &[state, animation] : animations)
    {
        animation.reset();
    }
}

void AnimationManager::clear()
{
    animations.clear();
}