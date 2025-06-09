#include "animations/animation_manager.hpp"
#include "game/player/player_state.hpp"

void AnimationManager::update(float deltaTime, const PlayerState &playerState)
{
    bool isWalking = std::abs(playerState.velocity.x) > 0.001f;
    PlayerAnimationState newState = isWalking ? PlayerAnimationState::Walk : PlayerAnimationState::Idle;
    if (newState != currentState)
    {
        currentState = newState;
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
}