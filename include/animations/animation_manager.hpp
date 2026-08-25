#pragma once

#include <unordered_map>
#include "game/actor/actor_animation_state.hpp"
#include "animations/sprite_animation.hpp"

struct AgentState;

class AnimationManager
{
public:
    void update(float deltaTime, const AgentState &agentState);
    const SpriteAnimation &getCurrentAnimation();
    void addAnimation(ActorAnimationState state, const SpriteAnimation &anim);
    ActorAnimationState getCurrentState() const;

private:
    ActorAnimationState currentState = ActorAnimationState::Idle;
    std::unordered_map<ActorAnimationState, SpriteAnimation> animations;
};