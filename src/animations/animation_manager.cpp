#include "animations/animation_manager.hpp"
#include "agent/agent_state.hpp"

void AnimationManager::update(float deltaTime, const AgentState &agentState)
{
    ActorAnimationState newState = currentState;

    if (agentState.dashing)
        newState = ActorAnimationState::Dash;
    else if (!agentState.onGround)
    {
        if (agentState.wallSliding || agentState.climbing)
            newState = ActorAnimationState::WallSlide;
        else if (agentState.velocity.y < 0.0f)
            newState = ActorAnimationState::Jump;
        else if (agentState.velocity.y > 0.0f)
            newState = ActorAnimationState::Fall;
    }
    else if (std::abs(agentState.velocity.x) > 0.1f)
        newState = ActorAnimationState::Walk;
    else
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