#pragma once

#include <unordered_map>
#include "game/player/player_animation_state.hpp"
#include "animations/sprite_animation.hpp"

struct AgentState;

class AnimationManager
{
public:
    void update(float deltaTime, const AgentState &agentState);
    const SpriteAnimation &getCurrentAnimation();
    void addAnimation(PlayerAnimationState state, const SpriteAnimation &anim);
    PlayerAnimationState getCurrentState() const;

private:
    PlayerAnimationState currentState = PlayerAnimationState::Idle;
    std::unordered_map<PlayerAnimationState, SpriteAnimation> animations;
};