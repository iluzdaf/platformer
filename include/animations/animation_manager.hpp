#pragma once

#include <unordered_map>
#include "actor/actor_animation_state.hpp"
#include "animations/frame_animation.hpp"

struct ActorMotionState;

class AnimationManager
{
public:
    void update(float deltaTime, const ActorMotionState &motionState);
    const FrameAnimation &getCurrentAnimation();
    void addAnimation(ActorAnimationState state, const FrameAnimation &anim);
    ActorAnimationState getCurrentState() const;

private:
    ActorAnimationState currentState = ActorAnimationState::Idle;
    std::unordered_map<ActorAnimationState, FrameAnimation> animations;
};
