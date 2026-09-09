#include "animations/animator.hpp"
#include "actor/observed.hpp"
#include "actor/actor_animation_state.hpp"
#include "actor/decided.hpp"
#include <cstdlib>
#include "animations/frame_animation.hpp"
#include <string>
#include <vector>

void Animator::animate(float deltaTime, const Decided &decided, const Observed &observed)
{
    ActorAnimationState newState = currentState;

    if (!observed.alive)
        newState = ActorAnimationState::Dead;
    else if (decided.knockback.active)
        newState = ActorAnimationState::Knockback;
    else if (decided.swing.swinging())
        newState = ActorAnimationState::Attack;
    else if (decided.dash.active)
        newState = ActorAnimationState::Dash;
    else if (!observed.contacts.onGround)
    {
        if (decided.wallHang.active && decided.wallClimb.velocity.y != 0.0f)
            newState = ActorAnimationState::Climb;
        else if (decided.wallSlide.active || decided.wallHang.active)
            newState = ActorAnimationState::WallSlide;
        else if (observed.velocity.y < 0.0f)
            newState = ActorAnimationState::Jump;
        else if (observed.velocity.y > 0.0f)
            newState = ActorAnimationState::Fall;
    }
    else if (std::abs(observed.velocity.x) > 0.1f)
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

const FrameAnimation &Animator::playing() const
{
    return animations.at(currentState);
}

ActorAnimationState Animator::state() const
{
    return currentState;
}

void Animator::add(ActorAnimationState state, const FrameAnimation &animation)
{
    animations.insert_or_assign(state, animation);
}
std::vector<std::string> Animator::takeCues()
{
    auto playingNow = animations.find(currentState);
    return playingNow == animations.end() ? std::vector<std::string>{}
                                          : playingNow->second.takeCues();
}

bool Animator::finished() const
{
    auto playingNow = animations.find(currentState);
    return playingNow != animations.end() && playingNow->second.finished();
}
