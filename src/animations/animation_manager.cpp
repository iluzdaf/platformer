#include "animations/animation_manager.hpp"
#include "agent/agent_state.hpp"

void AnimationManager::update(float deltaTime, const AgentState &agentState)
{
    PlayerAnimationState newState = currentState;

    if (agentState.dashing)
        newState = PlayerAnimationState::Dash;
    else if (!agentState.onGround)
    {
        if (agentState.wallSliding || agentState.climbing)
            newState = PlayerAnimationState::WallSlide;
        else if (agentState.velocity.y < 0.0f)
            newState = PlayerAnimationState::Jump;
        else if (agentState.velocity.y > 0.0f)
            newState = PlayerAnimationState::Fall;
    }
    else if (std::abs(agentState.velocity.x) > 0.1f)
        newState = PlayerAnimationState::Walk;
    else
        newState = PlayerAnimationState::Idle;

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